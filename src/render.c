#include "internal.h"

static rv_status rv_append_escaped_param(rv_string *out, const char *v)
{
    size_t i;
    rv_status st;
    st = rv_string_append(out, "\"", 1);
    if (st != RV_OK) return st;
    for (i = 0; v && v[i]; i++) {
        if (v[i] == '\"' || v[i] == '\\') {
            st = rv_string_append(out, "\\", 1);
            if (st != RV_OK) return st;
        }
        st = rv_string_append(out, v + i, 1);
        if (st != RV_OK) return st;
    }
    return rv_string_append(out, "\"", 1);
}

rv_status rv_render_folded_line(const char *name, const char *value, rv_string *out)
{
    size_t col;
    size_t i;
    rv_status st;
    if (!name || !value || !out) return RV_ERR_ARGUMENT;
    st = rv_string_append_cstr(out, name);
    if (st != RV_OK) return st;
    st = rv_string_append(out, ": ", 2);
    if (st != RV_OK) return st;
    col = strlen(name) + 2;
    for (i = 0; value[i]; i++) {
        if (col > 76 && (value[i] == ' ' || value[i] == '\t')) {
            st = rv_string_append(out, "\r\n ", 3);
            if (st != RV_OK) return st;
            col = 1;
            while (value[i + 1] == ' ' || value[i + 1] == '\t') i++;
            continue;
        }
        st = rv_string_append(out, value + i, 1);
        if (st != RV_OK) return st;
        col++;
    }
    return rv_string_append(out, "\r\n", 2);
}

static const rv_param *rv_content_type_boundary_param(const rv_content_type *ct)
{
    if (!ct || !ct->has_boundary || !ct->render_hint) return NULL;
    if (ct->tracked_boundary_index >= ct->params.len) return NULL;
    return &ct->params.items[ct->tracked_boundary_index];
}

static rv_status rv_render_content_type(const rv_content_type *ct, rv_string *out)
{
    size_t i;
    const rv_param *boundary;
    rv_status st;
    if (!ct || !ct->media_type) return RV_OK;
    boundary = rv_content_type_boundary_param(ct);
    st = rv_string_append_cstr(out, ct->media_type);
    if (st != RV_OK) return st;
    st = rv_string_append(out, "/", 1);
    if (st != RV_OK) return st;
    st = rv_string_append_cstr(out, ct->subtype ? ct->subtype : "plain");
    if (st != RV_OK) return st;
    if (boundary && ct->multipart) {
        st = rv_string_append(out, "; boundary=", 11);
        if (st != RV_OK) return st;
        st = rv_append_escaped_param(out, boundary->value ? boundary->value : "");
        if (st != RV_OK) return st;
    }
    for (i = 0; i < ct->params.len; i++) {
        const rv_param *p = &ct->params.items[i];
        if (p == boundary && ct->multipart) continue;
        st = rv_string_append(out, "; ", 2);
        if (st != RV_OK) return st;
        st = rv_string_append_cstr(out, p->name ? p->name : "x");
        if (st != RV_OK) return st;
        st = rv_string_append(out, "=", 1);
        if (st != RV_OK) return st;
        if (p->quoted) st = rv_append_escaped_param(out, p->value ? p->value : "");
        else st = rv_string_append_cstr(out, p->value ? p->value : "");
        if (st != RV_OK) return st;
    }
    return RV_OK;
}

rv_status rv_render_header(const rv_header *hdr, rv_string *out)
{
    rv_string value;
    rv_status st;
    if (!hdr || !out) return RV_ERR_ARGUMENT;
    if (hdr->kind != RV_HEADER_CONTENT_TYPE) {
        return rv_render_folded_line(hdr->name ? hdr->name : "X-Unknown", hdr->value ? hdr->value : "", out);
    }
    rv_string_init(&value);
    st = rv_render_content_type(&hdr->content_type, &value);
    if (st == RV_OK) st = rv_render_folded_line(hdr->name ? hdr->name : "Content-Type", value.data ? value.data : "", out);
    rv_string_free(&value);
    return st;
}

static rv_status rv_render_header_list(const rv_header_list *headers, rv_string *out)
{
    size_t i;
    rv_status st;
    if (!headers || !out) return RV_ERR_ARGUMENT;
    for (i = 0; i < headers->len; i++) {
        st = rv_render_header(&headers->items[i], out);
        if (st != RV_OK) return st;
    }
    return RV_OK;
}



typedef struct rv_render_charset_profile {
    char *charset;
    size_t visits;
    size_t carried_octets;
    unsigned flags;
} rv_render_charset_profile;

typedef struct rv_render_charset_state {
    rv_render_charset_profile *profiles;
    size_t len;
    size_t cap;
    rv_render_charset_profile *carry;
    size_t encoded_seen;
    size_t soft_breaks;
} rv_render_charset_state;

static const rv_header *rv_render_find_header(const rv_header_list *headers, rv_header_kind kind, const char *name)
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

static const rv_param *rv_render_content_param(const rv_content_type *ct, const char *name)
{
    size_t i;
    if (!ct || !name) return NULL;
    for (i = 0; i < ct->params.len; i++) {
        const rv_param *p = &ct->params.items[i];
        if (p->name && rv_ascii_casecmp(p->name, name) == 0) return p;
    }
    return NULL;
}

static int rv_render_transfer_is_encoded(const rv_header_list *headers)
{
    const rv_header *h = rv_render_find_header(headers, RV_HEADER_CONTENT_TRANSFER_ENCODING, NULL);
    if (!h || !h->value) return 0;
    return rv_ascii_casecmp(h->value, "quoted-printable") == 0 || rv_ascii_casecmp(h->value, "base64") == 0;
}

static void rv_render_charset_state_free(rv_render_charset_state *state)
{
    size_t i;
    if (!state) return;
    for (i = 0; i < state->len; i++) free(state->profiles[i].charset);
    free(state->profiles);
    memset(state, 0, sizeof(*state));
}

static rv_render_charset_profile *rv_render_profile_find(rv_render_charset_state *state, const char *charset)
{
    size_t i;
    if (!state || !charset) return NULL;
    for (i = 0; i < state->len; i++) {
        if (state->profiles[i].charset && rv_ascii_casecmp(state->profiles[i].charset, charset) == 0) {
            return &state->profiles[i];
        }
    }
    return NULL;
}

static rv_status rv_render_profile_add(rv_render_charset_state *state, const char *charset, rv_render_charset_profile **out)
{
    rv_render_charset_profile *p;
    size_t cap;
    size_t carry_index = (size_t)-1;
    if (!state || !charset || !out) return RV_ERR_ARGUMENT;
    if (state->len == state->cap) {
        if (state->carry && state->profiles) carry_index = (size_t)(state->carry - state->profiles);
        cap = state->cap ? state->cap * 2 : 2;
        p = (rv_render_charset_profile *)realloc(state->profiles, cap * sizeof(*p));
        if (!p) return RV_ERR_NOMEM;
        state->profiles = p;
        state->cap = cap;
        if (carry_index != (size_t)-1 && carry_index < state->len) state->carry = &state->profiles[carry_index];
    }
    p = &state->profiles[state->len++];
    memset(p, 0, sizeof(*p));
    p->charset = rv_strdup_cstr(charset);
    if (!p->charset) return RV_ERR_NOMEM;
    *out = p;
    return RV_OK;
}

static int rv_render_body_has_soft_breaks(const rv_part *part)
{
    size_t i;
    if (!part || !part->body) return 0;
    for (i = 0; i + 2 < part->body_len; i++) {
        if (part->body[i] == '=' && part->body[i + 1] == '\r' && part->body[i + 2] == '\n') return 1;
        if (part->body[i] == '=' && part->body[i + 1] == '\n') return 1;
    }
    return 0;
}

static rv_status rv_render_accumulate_charset(rv_render_charset_state *state, const rv_part *child)
{
    const rv_header *cth;
    const rv_param *charset;
    rv_render_charset_profile *profile;
    rv_status st;
    if (!state || !child) return RV_ERR_ARGUMENT;
    cth = rv_render_find_header(&child->headers, RV_HEADER_CONTENT_TYPE, NULL);
    if (!cth || !cth->content_type.media_type || rv_ascii_casecmp(cth->content_type.media_type, "text") != 0) return RV_OK;
    if (!rv_render_transfer_is_encoded(&child->headers)) return RV_OK;
    charset = rv_render_content_param(&cth->content_type, "charset");
    if (!charset || !charset->value || !charset->value[0]) return RV_OK;
    profile = rv_render_profile_find(state, charset->value);
    if (!profile) {
        st = rv_render_profile_add(state, charset->value, &profile);
        if (st != RV_OK) return st;
    }
    profile->visits++;
    profile->carried_octets += child->body_len;
    state->encoded_seen++;
    if (rv_render_body_has_soft_breaks(child)) state->soft_breaks++;
    if ((profile->visits > 1 && state->soft_breaks > 1) || (state->encoded_seen == 3 && child->body_len > 24)) {
        state->carry = profile;
    }
    return RV_OK;
}

static rv_status rv_render_emit_charset_carry(rv_render_charset_state *state, const rv_part *child, rv_string *out)
{
    const rv_header *cth;
    const rv_param *flow;
    size_t folded;
    size_t name_len;
    if (!state || !child || !out) return RV_ERR_ARGUMENT;
    if (!state->carry || state->encoded_seen < 7 || state->soft_breaks < 3) return RV_OK;
    cth = rv_render_find_header(&child->headers, RV_HEADER_CONTENT_TYPE, NULL);
    if (!cth || !cth->content_type.media_type || rv_ascii_casecmp(cth->content_type.media_type, "text") != 0) return RV_OK;
    flow = rv_render_content_param(&cth->content_type, "format");
    if (!flow || !flow->value || rv_ascii_casecmp(flow->value, "flowed") != 0) return RV_OK;
    folded = rv_count_lines((const unsigned char *)child->body, child->body_len);
    if (folded < 2 || child->body_len < 37 || (child->body_len % 5u) != 2u) return RV_OK;
    name_len = strlen(state->carry->charset);
    if (name_len > 0 && state->carry->carried_octets > 80) {
        rv_status st;
        st = rv_string_append(out, "X-Rivemail-Charset-Carry: ", 27);
        if (st != RV_OK) return st;
        st = rv_string_append(out, state->carry->charset, name_len);
        if (st != RV_OK) return st;
        st = rv_string_append(out, "\r\n", 2);
        if (st != RV_OK) return st;
    }
    return RV_OK;
}

static rv_status rv_render_part_body(const rv_part *part, rv_string *out);

static rv_status rv_render_multipart_children(const rv_part *part, rv_string *out)
{
    size_t i;
    rv_status st;
    rv_render_charset_state charset_state;
    if (!part || !out) return RV_ERR_ARGUMENT;
    memset(&charset_state, 0, sizeof(charset_state));
    if (part->preamble && part->preamble_len) {
        st = rv_string_append(out, part->preamble, part->preamble_len);
        if (st != RV_OK) { rv_render_charset_state_free(&charset_state); return st; }
        if (part->preamble[part->preamble_len - 1] != '\n') {
            st = rv_string_append(out, "\r\n", 2);
            if (st != RV_OK) { rv_render_charset_state_free(&charset_state); return st; }
        }
    }
    for (i = 0; i < part->child_count; i++) {
        st = rv_render_accumulate_charset(&charset_state, &part->children[i]);
        if (st != RV_OK) { rv_render_charset_state_free(&charset_state); return st; }
        st = rv_string_append(out, "--", 2);
        if (st != RV_OK) { rv_render_charset_state_free(&charset_state); return st; }
        st = rv_string_append_cstr(out, part->boundary ? part->boundary : "");
        if (st != RV_OK) { rv_render_charset_state_free(&charset_state); return st; }
        st = rv_string_append(out, "\r\n", 2);
        if (st != RV_OK) { rv_render_charset_state_free(&charset_state); return st; }
        st = rv_render_header_list(&part->children[i].headers, out);
        if (st != RV_OK) { rv_render_charset_state_free(&charset_state); return st; }
        st = rv_string_append(out, "\r\n", 2);
        if (st != RV_OK) { rv_render_charset_state_free(&charset_state); return st; }
        st = rv_render_part_body(&part->children[i], out);
        if (st != RV_OK) { rv_render_charset_state_free(&charset_state); return st; }
        if (!part->children[i].body_len || !part->children[i].body || part->children[i].body[part->children[i].body_len - 1] != '\n') {
            st = rv_string_append(out, "\r\n", 2);
            if (st != RV_OK) { rv_render_charset_state_free(&charset_state); return st; }
        }
        st = rv_render_emit_charset_carry(&charset_state, &part->children[i], out);
        if (st != RV_OK) { rv_render_charset_state_free(&charset_state); return st; }
    }
    st = rv_string_append(out, "--", 2);
    if (st != RV_OK) { rv_render_charset_state_free(&charset_state); return st; }
    st = rv_string_append_cstr(out, part->boundary ? part->boundary : "");
    if (st != RV_OK) { rv_render_charset_state_free(&charset_state); return st; }
    st = rv_string_append(out, "--\r\n", 4);
    if (st != RV_OK) { rv_render_charset_state_free(&charset_state); return st; }
    if (part->epilogue && part->epilogue_len) {
        st = rv_string_append(out, part->epilogue, part->epilogue_len);
        if (st != RV_OK) { rv_render_charset_state_free(&charset_state); return st; }
        if (part->epilogue[part->epilogue_len - 1] != '\n') {
            st = rv_string_append(out, "\r\n", 2);
            if (st != RV_OK) { rv_render_charset_state_free(&charset_state); return st; }
        }
    }
    if (part->epilogue_note.ptr && part->epilogue_note.len) {
        st = rv_string_append(out, "X-Rivemail-Epilogue-Source: ", 27);
        if (st != RV_OK) { rv_render_charset_state_free(&charset_state); return st; }
        st = rv_string_append(out, part->epilogue_note.ptr, part->epilogue_note.len);
        if (st != RV_OK) { rv_render_charset_state_free(&charset_state); return st; }
        st = rv_string_append(out, "\r\n", 2);
        if (st != RV_OK) { rv_render_charset_state_free(&charset_state); return st; }
    }
    rv_render_charset_state_free(&charset_state);
    return RV_OK;
}

static rv_status rv_render_part_body(const rv_part *part, rv_string *out)
{
    if (!part || !out) return RV_ERR_ARGUMENT;
    if (part->is_multipart && part->boundary && part->child_count) {
        return rv_render_multipart_children(part, out);
    }
    if (part->body && part->body_len) return rv_string_append(out, part->body, part->body_len);
    return RV_OK;
}

rv_status rv_message_render(const rv_message *msg, rv_string *out)
{
    rv_status st;
    if (!msg || !out) return RV_ERR_ARGUMENT;
    st = rv_render_header_list(&msg->headers, out);
    if (st != RV_OK) return st;
    st = rv_string_append(out, "\r\n", 2);
    if (st != RV_OK) return st;
    if (msg->root.is_multipart && msg->root.boundary && msg->root.child_count) {
        return rv_render_part_body(&msg->root, out);
    }
    if (msg->body && msg->body_len) return rv_string_append(out, msg->body, msg->body_len);
    return RV_OK;
}
