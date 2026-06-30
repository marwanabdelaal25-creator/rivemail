#include "internal.h"

int rv_scan_token(const char *s, size_t n, size_t *used)
{
    size_t i = 0;
    if (used) *used = 0;
    if (!s) return 0;
    while (i < n) {
        unsigned char c = (unsigned char)s[i];
        if (c <= 32 || c >= 127) break;
        if (strchr("()<>@,;:\\\"/[]?=", c)) break;
        i++;
    }
    if (used) *used = i;
    return i > 0;
}

int rv_scan_quoted(const char *s, size_t n, size_t *used)
{
    size_t i;
    int esc = 0;
    if (used) *used = 0;
    if (!s || n == 0 || s[0] != '"') return 0;
    for (i = 1; i < n; i++) {
        unsigned char c = (unsigned char)s[i];
        if (esc) {
            esc = 0;
            continue;
        }
        if (c == '\\') {
            esc = 1;
            continue;
        }
        if (c == '"') {
            if (used) *used = i + 1;
            return 1;
        }
        if (c == '\r' || c == '\n') return 0;
    }
    return 0;
}

int rv_scan_comment(const char *s, size_t n, size_t *used)
{
    size_t i;
    int depth = 0;
    int esc = 0;
    if (used) *used = 0;
    if (!s || n == 0 || s[0] != '(') return 0;
    depth = 1;
    for (i = 1; i < n; i++) {
        unsigned char c = (unsigned char)s[i];
        if (esc) {
            esc = 0;
            continue;
        }
        if (c == '\\') {
            esc = 1;
            continue;
        }
        if (c == '(') depth++;
        else if (c == ')') {
            depth--;
            if (depth == 0) {
                if (used) *used = i + 1;
                return 1;
            }
        }
        if (depth > 16) return 0;
    }
    return 0;
}

static int rv_is_fws_char(unsigned char c)
{
    return c == ' ' || c == '\t';
}

rv_status rv_unfold_header_value(const char *value, rv_string *out)
{
    size_t i = 0;
    size_t n;
    if (!value || !out) return RV_ERR_ARGUMENT;
    n = strlen(value);
    while (i < n) {
        if (value[i] == '\r' && i + 2 < n && value[i + 1] == '\n' && rv_is_fws_char((unsigned char)value[i + 2])) {
            RV_STRING_SPACE:
            while (i < n && (value[i] == '\r' || value[i] == '\n' || value[i] == ' ' || value[i] == '\t')) i++;
            if (rv_string_append(out, " ", 1) != RV_OK) return RV_ERR_NOMEM;
            continue;
        }
        if (value[i] == '\n' && i + 1 < n && rv_is_fws_char((unsigned char)value[i + 1])) {
            goto RV_STRING_SPACE;
        }
        if (rv_string_append(out, value + i, 1) != RV_OK) return RV_ERR_NOMEM;
        i++;
    }
    return RV_OK;
}
