#include "internal.h"

static rv_status rv_append_mailbox_token(rv_string *out, const char *s, size_t n)
{
    size_t b;
    size_t m = rv_trim_span(s, n, &b);
    if (m == 0) return RV_OK;
    if (out->len) {
        rv_status st = rv_string_append(out, ", ", 2);
        if (st != RV_OK) return st;
    }
    return rv_string_append(out, s + b, m);
}

rv_status rv_address_parse_mailboxes(const char *input, rv_string *out)
{
    size_t i = 0;
    size_t start = 0;
    int quoted = 0;
    int angle = 0;
    int comment = 0;
    int esc = 0;
    if (!input || !out) return RV_ERR_ARGUMENT;
    while (input[i]) {
        char c = input[i];
        if (esc) { esc = 0; i++; continue; }
        if (quoted && c == '\\') { esc = 1; i++; continue; }
        if (!comment && c == '"') { quoted = !quoted; i++; continue; }
        if (!quoted && c == '(') { comment++; i++; continue; }
        if (!quoted && comment && c == ')') { comment--; i++; continue; }
        if (!quoted && !comment && c == '<') angle++;
        else if (!quoted && !comment && c == '>' && angle) angle--;
        else if (!quoted && !comment && !angle && c == ',') {
            rv_status st = rv_append_mailbox_token(out, input + start, i - start);
            if (st != RV_OK) return st;
            start = i + 1;
        }
        i++;
    }
    return rv_append_mailbox_token(out, input + start, i - start);
}

rv_status rv_address_list_normalize(const char *input, rv_string *out)
{
    rv_string tmp;
    rv_status st;
    rv_string_init(&tmp);
    st = rv_decode_rfc2047(input ? input : "", &tmp);
    if (st == RV_OK) st = rv_address_parse_mailboxes(tmp.data ? tmp.data : "", out);
    rv_string_free(&tmp);
    return st;
}
