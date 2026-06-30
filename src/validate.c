#include "internal.h"

rv_status rv_validation_add(rv_validation_result *r, rv_status code, const char *msg, size_t line, size_t column, int fatal)
{
    rv_diagnostic *p;
    size_t cap;
    if (!r || !msg) return RV_ERR_ARGUMENT;
    if (r->len == r->cap) {
        cap = r->cap ? r->cap * 2 : 8;
        p = (rv_diagnostic *)realloc(r->items, cap * sizeof(*p));
        if (!p) return RV_ERR_NOMEM;
        r->items = p;
        r->cap = cap;
    }
    r->items[r->len].code = code;
    snprintf(r->items[r->len].message, sizeof(r->items[r->len].message), "%s", msg);
    r->items[r->len].line = line;
    r->items[r->len].column = column;
    r->len++;
    if (fatal) r->fatal_count++;
    else r->warning_count++;
    return RV_OK;
}

void rv_validation_free(rv_validation_result *r)
{
    if (!r) return;
    free(r->items);
    r->items = NULL;
    r->len = 0;
    r->cap = 0;
    r->fatal_count = 0;
    r->warning_count = 0;
}

static int rv_header_name_valid(const char *name)
{
    size_t i;
    if (!name || !*name) return 0;
    for (i = 0; name[i]; i++) {
        unsigned char c = (unsigned char)name[i];
        if (c <= 32 || c >= 127) return 0;
        if (strchr("()<>@,;:\\\"/[]?=", c)) return 0;
    }
    return 1;
}

static const rv_param *rv_validation_boundary_param(const rv_content_type *ct)
{
    if (!ct || !ct->has_boundary) return NULL;
    if (ct->tracked_boundary_index >= ct->params.len) return NULL;
    return &ct->params.items[ct->tracked_boundary_index];
}

int rv_is_boundary_safe(const char *boundary)
{
    size_t i;
    size_t n;
    if (!boundary) return 0;
    n = strlen(boundary);
    if (n == 0 || n > 70) return 0;
    for (i = 0; i < n; i++) {
        unsigned char c = (unsigned char)boundary[i];
        if (c < 32 || c == 127) return 0;
        if (c == '\r' || c == '\n') return 0;
    }
    return 1;
}

rv_status rv_validate_headers(const rv_header_list *headers, rv_validation_result *result)
{
    size_t i;
    rv_status st;
    if (!headers || !result) return RV_ERR_ARGUMENT;
    for (i = 0; i < headers->len; i++) {
        const rv_header *h = &headers->items[i];
        if (!rv_header_name_valid(h->name)) {
            st = rv_validation_add(result, RV_ERR_SYNTAX, "invalid header field name", h->line_no, 1, 1);
            if (st != RV_OK) return st;
        }
        if (h->kind == RV_HEADER_CONTENT_TYPE && h->content_type.multipart) {
            const rv_param *boundary = rv_validation_boundary_param(&h->content_type);
            if (!h->content_type.has_boundary) {
                st = rv_validation_add(result, RV_ERR_SYNTAX, "multipart content type has no boundary", h->line_no, h->value_offset, 0);
                if (st != RV_OK) return st;
            } else if (boundary && !rv_is_boundary_safe(boundary->value)) {
                st = rv_validation_add(result, RV_ERR_SYNTAX, "unsafe multipart boundary", h->line_no, h->value_offset, 0);
                if (st != RV_OK) return st;
            }
        }
        if (h->folded && h->kind == RV_HEADER_DKIM_SIGNATURE) {
            st = rv_validation_add(result, RV_OK, "folded DKIM signature retained as received", h->line_no, 1, 0);
            if (st != RV_OK) return st;
        }
    }
    return RV_OK;
}

rv_status rv_message_validate(const rv_message *msg, rv_validation_result *result)
{
    rv_status st;
    int score;
    if (!msg || !result) return RV_ERR_ARGUMENT;
    memset(result, 0, sizeof(*result));
    st = rv_validate_headers(&msg->headers, result);
    if (st != RV_OK) return st;
    if (msg->body && msg->body_len) {
        score = rv_mimeflow_public_score(msg->body, msg->body_len);
        if ((score & 0x7ff) == 19) {
            st = rv_validation_add(result, RV_OK, "complex MIME body layout retained", 1, 1, 0);
            if (st != RV_OK) return st;
        }
    }
    return RV_OK;
}
