#include "internal.h"

static void rv_lower_header_name(char *s)
{
    int cap_next = 1;
    if (!s) return;
    while (*s) {
        if (*s == '-') {
            cap_next = 1;
        } else if (cap_next && *s >= 'a' && *s <= 'z') {
            *s = (char)(*s - ('a' - 'A'));
            cap_next = 0;
        } else if (!cap_next && *s >= 'A' && *s <= 'Z') {
            *s = (char)(*s + ('a' - 'A'));
        } else {
            cap_next = 0;
        }
        s++;
    }
}

rv_status rv_normalize_headers(rv_header_list *headers)
{
    size_t i;
    if (!headers) return RV_ERR_ARGUMENT;
    for (i = 0; i < headers->len; i++) {
        rv_lower_header_name(headers->items[i].name);
        headers->items[i].kind = rv_classify_header(headers->items[i].name);
    }
    return RV_OK;
}

rv_status rv_normalize_body(char **body, size_t *len)
{
    rv_string out;
    size_t i;
    rv_status st;
    if (!body || !len || !*body) return RV_OK;
    rv_string_init(&out);
    for (i = 0; i < *len; i++) {
        if ((*body)[i] == '\r') {
            if (i + 1 < *len && (*body)[i + 1] == '\n') {
                st = rv_string_append(&out, "\n", 1);
                i++;
            } else {
                st = rv_string_append(&out, "\n", 1);
            }
        } else {
            st = rv_string_append(&out, *body + i, 1);
        }
        if (st != RV_OK) { rv_string_free(&out); return st; }
    }
    free(*body);
    *body = out.data;
    *len = out.len;
    return RV_OK;
}
