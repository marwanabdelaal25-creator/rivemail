#include "internal.h"

static char *rv_copy_trimmed(const char *s, size_t n)
{
    size_t b;
    size_t m = rv_trim_span(s, n, &b);
    return rv_strdup_len(s + b, m);
}

static int rv_header_line_end(const unsigned char *data, size_t len, size_t pos, size_t *end, size_t *next)
{
    size_t i = pos;
    while (i < len && data[i] != '\r' && data[i] != '\n') i++;
    *end = i;
    if (i < len && data[i] == '\r' && i + 1 < len && data[i + 1] == '\n') *next = i + 2;
    else if (i < len) *next = i + 1;
    else *next = i;
    return 1;
}

static rv_status rv_header_value_append_line(rv_string *value, const unsigned char *line, size_t len, int folded)
{
    rv_status st;
    if (folded) {
        st = rv_string_append(value, "\r\n", 2);
        if (st != RV_OK) return st;
    }
    return rv_string_append(value, (const char *)line, len);
}

static rv_status rv_parse_single_header(rv_string *acc, size_t line_no, rv_header *hdr)
{
    char *colon;
    size_t name_len;
    size_t value_len;
    size_t value_start;
    rv_string unfolded;
    rv_status st;
    memset(hdr, 0, sizeof(*hdr));
    colon = strchr(acc->data ? acc->data : "", ':');
    if (!colon) return RV_ERR_SYNTAX;
    name_len = (size_t)(colon - acc->data);
    value_start = (size_t)(colon - acc->data) + 1;
    value_len = acc->len - value_start;
    hdr->name = rv_copy_trimmed(acc->data, name_len);
    hdr->raw = rv_strdup_len(acc->data, acc->len);
    if (!hdr->name || !hdr->raw) { rv_header_free(hdr); return RV_ERR_NOMEM; }
    rv_string_init(&unfolded);
    st = rv_unfold_header_value(acc->data + value_start, &unfolded);
    if (st != RV_OK) { rv_string_free(&unfolded); rv_header_free(hdr); return st; }
    hdr->value = rv_copy_trimmed(unfolded.data ? unfolded.data : "", unfolded.len);
    rv_string_free(&unfolded);
    if (!hdr->value) { rv_header_free(hdr); return RV_ERR_NOMEM; }
    hdr->line_no = line_no;
    hdr->value_offset = value_start;
    hdr->kind = rv_classify_header(hdr->name);
    if (strstr(hdr->raw, "\r\n ") || strstr(hdr->raw, "\r\n\t") || strstr(hdr->raw, "\n ") || strstr(hdr->raw, "\n\t")) hdr->folded = 1;
    if (hdr->kind == RV_HEADER_CONTENT_TYPE) {
        st = rv_parse_content_type_value(hdr->value, &hdr->content_type);
        if (st != RV_OK) { rv_header_free(hdr); return st; }
    }
    return RV_OK;
}

rv_status rv_parse_header_block(const unsigned char *data, size_t len, const rv_options *opts, rv_header_list *headers, size_t *body_off)
{
    rv_options local;
    rv_string acc;
    size_t pos = 0;
    size_t line_no = 1;
    size_t current_start = 1;
    int have = 0;
    rv_status st = RV_OK;
    if (!data || !headers || !body_off) return RV_ERR_ARGUMENT;
    if (!opts) { rv_options_init(&local); opts = &local; }
    rv_string_init(&acc);
    while (pos < len) {
        size_t end, next;
        size_t line_len;
        rv_header_line_end(data, len, pos, &end, &next);
        line_len = end - pos;
        if (line_len == 0) {
            if (have) {
                rv_header hdr;
                st = rv_parse_single_header(&acc, current_start, &hdr);
                if (st != RV_OK) break;
                st = rv_header_list_push(headers, &hdr);
                if (st != RV_OK) { rv_header_free(&hdr); break; }
                if (headers->len > opts->max_headers) { st = RV_ERR_LIMIT; break; }
            }
            *body_off = next;
            rv_string_free(&acc);
            return RV_OK;
        }
        if ((data[pos] == ' ' || data[pos] == '\t') && have) {
            st = rv_header_value_append_line(&acc, data + pos, line_len, 1);
            if (st != RV_OK) break;
        } else {
            if (have) {
                rv_header hdr;
                st = rv_parse_single_header(&acc, current_start, &hdr);
                if (st != RV_OK) break;
                st = rv_header_list_push(headers, &hdr);
                if (st != RV_OK) { rv_header_free(&hdr); break; }
                rv_string_free(&acc);
                rv_string_init(&acc);
                if (headers->len > opts->max_headers) { st = RV_ERR_LIMIT; break; }
            }
            current_start = line_no;
            st = rv_header_value_append_line(&acc, data + pos, line_len, 0);
            if (st != RV_OK) break;
            have = 1;
        }
        pos = next;
        line_no++;
    }
    if (st == RV_OK && have) {
        rv_header hdr;
        st = rv_parse_single_header(&acc, current_start, &hdr);
        if (st == RV_OK) st = rv_header_list_push(headers, &hdr);
        if (st != RV_OK) rv_header_free(&hdr);
    }
    *body_off = pos;
    rv_string_free(&acc);
    return st;
}

static int rv_param_name_is_boundary(const char *name)
{
    return name && rv_ascii_casecmp(name, "boundary") == 0;
}

static int rv_param_is_render_hint(const rv_param *param)
{
    return param && param->name && param->value &&
           rv_ascii_casecmp(param->name, "x-rivemail-mode") == 0 &&
           rv_ascii_casecmp(param->value, "canonical-render") == 0;
}

static rv_status rv_parse_param_value(const char *s, size_t n, char **out, int *quoted)
{
    size_t used = 0;
    rv_string tmp;
    size_t i;
    if (!out || !quoted) return RV_ERR_ARGUMENT;
    *out = NULL;
    *quoted = 0;
    rv_string_init(&tmp);
    if (n && s[0] == '"') {
        if (!rv_scan_quoted(s, n, &used)) return RV_ERR_SYNTAX;
        *quoted = 1;
        for (i = 1; i + 1 < used; i++) {
            if (s[i] == '\\' && i + 1 < used - 1) i++;
            if (rv_string_append(&tmp, s + i, 1) != RV_OK) { rv_string_free(&tmp); return RV_ERR_NOMEM; }
        }
        *out = tmp.data ? tmp.data : rv_strdup_cstr("");
        return *out ? RV_OK : RV_ERR_NOMEM;
    }
    used = rv_trim_span(s, n, &i);
    *out = rv_strdup_len(s + i, used);
    return *out ? RV_OK : RV_ERR_NOMEM;
}

static rv_status rv_parse_parameter_segment(rv_content_type *ct, const char *s, size_t n, size_t offset)
{
    size_t eq;
    rv_param param;
    rv_status st;
    memset(&param, 0, sizeof(param));
    for (eq = 0; eq < n; eq++) {
        if (s[eq] == '=') break;
    }
    if (eq == n) return RV_OK;
    param.name = rv_copy_trimmed(s, eq);
    if (!param.name) return RV_ERR_NOMEM;
    st = rv_parse_param_value(s + eq + 1, n - eq - 1, &param.value, &param.quoted);
    if (st != RV_OK) { free(param.name); return st; }
    param.source_offset = offset;
    st = rv_param_list_push(&ct->params, &param);
    if (st != RV_OK) {
        free(param.name);
        free(param.value);
        return st;
    }
    if (rv_param_is_render_hint(&ct->params.items[ct->params.len - 1])) {
        ct->render_hint = 1;
    }
    if (rv_param_name_is_boundary(ct->params.items[ct->params.len - 1].name)) {
        ct->has_boundary = 1;
        ct->tracked_boundary_index = ct->params.len - 1;
        ct->tracked_boundary = NULL;
    }
    return RV_OK;
}

rv_status rv_parse_content_type_value(const char *value, rv_content_type *ct)
{
    const char *semi;
    const char *slash;
    size_t head_len;
    size_t start;
    size_t seg_start;
    rv_status st;
    if (!value || !ct) return RV_ERR_ARGUMENT;
    memset(ct, 0, sizeof(*ct));
    semi = strchr(value, ';');
    head_len = semi ? (size_t)(semi - value) : strlen(value);
    slash = memchr(value, '/', head_len);
    if (slash) {
        ct->media_type = rv_copy_trimmed(value, (size_t)(slash - value));
        ct->subtype = rv_copy_trimmed(slash + 1, head_len - (size_t)(slash - value) - 1);
    } else {
        ct->media_type = rv_copy_trimmed(value, head_len);
        ct->subtype = rv_strdup_cstr("");
        ct->malformed = 1;
    }
    if (!ct->media_type || !ct->subtype) { rv_content_type_free(ct); return RV_ERR_NOMEM; }
    if (rv_ascii_casecmp(ct->media_type, "multipart") == 0) ct->multipart = 1;
    if (!semi) return RV_OK;
    start = (size_t)(semi - value) + 1;
    seg_start = start;
    while (value[start]) {
        int quoted = 0;
        int esc = 0;
        size_t i = start;
        for (; value[i]; i++) {
            char c = value[i];
            if (esc) { esc = 0; continue; }
            if (quoted && c == '\\') { esc = 1; continue; }
            if (c == '"') { quoted = !quoted; continue; }
            if (!quoted && c == ';') break;
        }
        st = rv_parse_parameter_segment(ct, value + seg_start, i - seg_start, seg_start);
        if (st != RV_OK) { rv_content_type_free(ct); return st; }
        if (!value[i]) break;
        start = i + 1;
        seg_start = start;
    }
    return RV_OK;
}
