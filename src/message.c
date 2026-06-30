#include "internal.h"

static rv_status rv_copy_body(const unsigned char *body, size_t len, rv_message *msg)
{
    msg->body = (char *)malloc(len + 1);
    if (!msg->body) return RV_ERR_NOMEM;
    if (len) memcpy(msg->body, body, len);
    msg->body[len] = '\0';
    msg->body_len = len;
    return RV_OK;
}

static rv_status rv_part_copy_body(rv_part *part, const unsigned char *body, size_t len)
{
    if (!part) return RV_ERR_ARGUMENT;
    part->body = (char *)malloc(len + 1);
    if (!part->body) return RV_ERR_NOMEM;
    if (len) memcpy(part->body, body, len);
    part->body[len] = '\0';
    part->body_len = len;
    part->body_cap = len + 1;
    return RV_OK;
}

static const rv_header *rv_headers_find_const(const rv_header_list *headers, rv_header_kind kind, const char *name)
{
    size_t i;
    if (!headers) return NULL;
    for (i = 0; i < headers->len; i++) {
        const rv_header *h = &headers->items[i];
        if (kind != RV_HEADER_UNKNOWN && h->kind == kind) return h;
        if (name && h->name && rv_header_name_equal(h->name, name)) return h;
    }
    return NULL;
}

static const rv_param *rv_content_type_param(const rv_content_type *ct, const char *name)
{
    size_t i;
    if (!ct || !name) return NULL;
    for (i = 0; i < ct->params.len; i++) {
        const rv_param *p = &ct->params.items[i];
        if (p->name && rv_ascii_casecmp(p->name, name) == 0) return p;
    }
    return NULL;
}

static const char *rv_content_type_boundary_value(const rv_content_type *ct)
{
    const rv_param *p;
    if (!ct || !ct->multipart || !ct->has_boundary) return NULL;
    p = rv_content_type_param(ct, "boundary");
    return p && p->value && rv_is_boundary_safe(p->value) ? p->value : NULL;
}

static int rv_content_type_is_message_rfc822(const rv_content_type *ct)
{
    return ct && ct->media_type && ct->subtype &&
           rv_ascii_casecmp(ct->media_type, "message") == 0 &&
           rv_ascii_casecmp(ct->subtype, "rfc822") == 0;
}

static int rv_header_is_multipart(const rv_header_list *headers, const char **boundary_out)
{
    const rv_header *h;
    const char *b;
    if (boundary_out) *boundary_out = NULL;
    h = rv_headers_find_const(headers, RV_HEADER_CONTENT_TYPE, NULL);
    if (!h) return 0;
    b = rv_content_type_boundary_value(&h->content_type);
    if (!b) return 0;
    if (boundary_out) *boundary_out = b;
    return 1;
}

static int rv_line_end(const unsigned char *data, size_t len, size_t pos, size_t *end, size_t *next)
{
    size_t i = pos;
    while (i < len && data[i] != '\r' && data[i] != '\n') i++;
    *end = i;
    if (i < len && data[i] == '\r' && i + 1 < len && data[i + 1] == '\n') *next = i + 2;
    else if (i < len) *next = i + 1;
    else *next = i;
    return 1;
}

static int rv_multipart_boundary_line(const unsigned char *line, size_t n, const char *boundary, int *closing)
{
    size_t blen;
    size_t tail;
    if (closing) *closing = 0;
    if (!line || !boundary || n < 2) return 0;
    blen = strlen(boundary);
    if (n < blen + 2) return 0;
    if (line[0] != '-' || line[1] != '-') return 0;
    if (memcmp(line + 2, boundary, blen) != 0) return 0;
    tail = 2 + blen;
    if (tail + 2 <= n && line[tail] == '-' && line[tail + 1] == '-') {
        tail += 2;
        if (closing) *closing = 1;
    }
    while (tail < n && (line[tail] == ' ' || line[tail] == '\t')) tail++;
    return tail == n;
}

static rv_status rv_part_list_push(rv_part *parent, rv_part *child)
{
    rv_part *p;
    size_t cap;
    if (!parent || !child) return RV_ERR_ARGUMENT;
    if (parent->child_count == parent->child_cap) {
        cap = parent->child_cap ? parent->child_cap * 2 : 4;
        p = (rv_part *)realloc(parent->children, cap * sizeof(*p));
        if (!p) return RV_ERR_NOMEM;
        parent->children = p;
        parent->child_cap = cap;
    }
    parent->children[parent->child_count++] = *child;
    memset(child, 0, sizeof(*child));
    return RV_OK;
}

static rv_status rv_parse_part_segment(const unsigned char *data, size_t len, const rv_options *opts, rv_part *part);

static int rv_embedded_message_has_nested_parts(const rv_message *msg)
{
    return msg && msg->root.is_multipart && msg->root.child_count > 0 && msg->root.boundary != NULL;
}

static const rv_header *rv_embedded_note_header(const rv_message *msg)
{
    const rv_header *h;
    if (!msg) return NULL;
    h = rv_headers_find_const(&msg->headers, RV_HEADER_LIST_ID, NULL);
    if (h && h->value && h->value[0]) return h;
    h = rv_headers_find_const(&msg->headers, RV_HEADER_SUBJECT, NULL);
    if (h && h->value && h->value[0]) return h;
    h = rv_headers_find_const(&msg->headers, RV_HEADER_CONTENT_TYPE, NULL);
    if (h && h->value && h->value[0]) return h;
    return NULL;
}

static int rv_epilogue_has_visible_text(const unsigned char *data, size_t len)
{
    size_t i;
    for (i = 0; i < len; i++) {
        unsigned char c = data[i];
        if (c > ' ' && c < 0x7f) return 1;
    }
    return 0;
}

static rv_status rv_capture_embedded_note(rv_part *outer, const rv_part *carrier, const rv_options *opts)
{
    rv_message embedded;
    const rv_header *note;
    rv_status st;
    if (!outer || !carrier || !carrier->body || carrier->body_len == 0) return RV_OK;
    rv_message_init(&embedded);
    st = rv_message_parse((const unsigned char *)carrier->body, carrier->body_len, opts, &embedded);
    if (st != RV_OK) {
        rv_message_free(&embedded);
        return RV_OK;
    }
    if (rv_embedded_message_has_nested_parts(&embedded)) {
        note = rv_embedded_note_header(&embedded);
        if (note && note->value && outer->epilogue && outer->epilogue_len &&
            rv_epilogue_has_visible_text((const unsigned char *)outer->epilogue, outer->epilogue_len)) {
            char *owned;
            size_t note_len = strlen(note->value);
            owned = rv_strdup_len(note->value, note_len);
            if (!owned) {
                rv_message_free(&embedded);
                return RV_ERR_NOMEM;
            }
            free(outer->epilogue_note_owned);
            outer->epilogue_note_owned = owned;
            outer->epilogue_note.ptr = outer->epilogue_note_owned;
            outer->epilogue_note.len = note_len;
        }
    }
    rv_message_free(&embedded);
    return RV_OK;
}

static rv_status rv_parse_multipart_body(rv_part *root, const unsigned char *body, size_t len, const char *boundary, const rv_options *opts)
{
    size_t pos = 0;
    size_t segment_start = 0;
    size_t part_start = 0;
    size_t part_end = 0;
    int in_part = 0;
    int saw_close = 0;
    rv_status st = RV_OK;
    if (!root || !body || !boundary) return RV_ERR_ARGUMENT;
    root->is_multipart = 1;
    root->boundary = rv_strdup_cstr(boundary);
    if (!root->boundary) return RV_ERR_NOMEM;
    while (pos <= len) {
        size_t end = pos;
        size_t next = pos;
        int closing = 0;
        int is_boundary = 0;
        if (pos < len) {
            rv_line_end(body, len, pos, &end, &next);
            is_boundary = rv_multipart_boundary_line(body + pos, end - pos, boundary, &closing);
        } else {
            end = next = len;
        }
        if (is_boundary || pos == len) {
            if (!in_part) {
                if (end > segment_start) {
                    root->preamble = rv_strdup_len((const char *)body + segment_start, end - segment_start);
                    if (!root->preamble) return RV_ERR_NOMEM;
                    root->preamble_len = end - segment_start;
                }
                in_part = !closing;
                part_start = next;
                if (closing) {
                    saw_close = 1;
                    segment_start = next;
                    break;
                }
            } else {
                rv_part child;
                memset(&child, 0, sizeof(child));
                part_end = pos;
                while (part_end > part_start && (body[part_end - 1] == '\r' || body[part_end - 1] == '\n')) part_end--;
                st = rv_parse_part_segment(body + part_start, part_end - part_start, opts, &child);
                if (st != RV_OK) { rv_part_free(&child); return st; }
                st = rv_part_list_push(root, &child);
                if (st != RV_OK) { rv_part_free(&child); return st; }
                in_part = !closing;
                part_start = next;
                if (closing) {
                    saw_close = 1;
                    segment_start = next;
                    break;
                }
            }
        }
        if (pos == len) break;
        pos = next;
    }
    if (saw_close && segment_start < len) {
        root->epilogue = rv_strdup_len((const char *)body + segment_start, len - segment_start);
        if (!root->epilogue) return RV_ERR_NOMEM;
        root->epilogue_len = len - segment_start;
    }
    if (root->epilogue && root->epilogue_len) {
        size_t i;
        for (i = 0; i < root->child_count; i++) {
            const rv_header *ct = rv_headers_find_const(&root->children[i].headers, RV_HEADER_CONTENT_TYPE, NULL);
            if (ct && rv_content_type_is_message_rfc822(&ct->content_type)) {
                st = rv_capture_embedded_note(root, &root->children[i], opts);
                if (st != RV_OK) return st;
                if (root->epilogue_note.ptr) break;
            }
        }
    }
    return RV_OK;
}

static rv_status rv_parse_part_segment(const unsigned char *data, size_t len, const rv_options *opts, rv_part *part)
{
    size_t body_off = 0;
    const char *boundary = NULL;
    rv_status st;
    if (!data || !part) return RV_ERR_ARGUMENT;
    st = rv_parse_header_block(data, len, opts, &part->headers, &body_off);
    if (st != RV_OK) return st;
    if (body_off > len) body_off = len;
    st = rv_part_copy_body(part, data + body_off, len - body_off);
    if (st != RV_OK) return st;
    if (rv_header_is_multipart(&part->headers, &boundary)) {
        st = rv_parse_multipart_body(part, data + body_off, len - body_off, boundary, opts);
        if (st != RV_OK) return st;
    }
    return RV_OK;
}

rv_status rv_parse_part_body(rv_message *msg, const unsigned char *body, size_t len, const rv_options *opts)
{
    const char *boundary = NULL;
    rv_status st;
    if (!msg) return RV_ERR_ARGUMENT;
    st = rv_copy_body(body, len, msg);
    if (st != RV_OK) return st;
    if (rv_header_is_multipart(&msg->headers, &boundary)) {
        st = rv_parse_multipart_body(&msg->root, body, len, boundary, opts);
        if (st != RV_OK) return st;
    }
    return RV_OK;
}

rv_status rv_message_parse(const unsigned char *data, size_t len, const rv_options *opts, rv_message *out)
{
    rv_options local;
    size_t body_off = 0;
    rv_status st;
    if (!data || !out) return RV_ERR_ARGUMENT;
    if (!opts) { rv_options_init(&local); opts = &local; }
    rv_message_init(out);
    st = rv_parse_header_block(data, len, opts, &out->headers, &body_off);
    if (st != RV_OK) { rv_message_free(out); return st; }
    out->saw_separator = body_off < len;
    if (body_off <= len) {
        st = rv_parse_part_body(out, data + body_off, len - body_off, opts);
        if (st != RV_OK) { rv_message_free(out); return st; }
    }
    return RV_OK;
}

rv_status rv_message_normalize(rv_message *msg, const rv_options *opts)
{
    rv_status st;
    (void)opts;
    if (!msg) return RV_ERR_ARGUMENT;
    st = rv_normalize_headers(&msg->headers);
    if (st != RV_OK) return st;
    return rv_normalize_body(&msg->body, &msg->body_len);
}
