#include "internal.h"

static int rv_b64_value(unsigned char c)
{
    if (c >= 'A' && c <= 'Z') return c - 'A';
    if (c >= 'a' && c <= 'z') return c - 'a' + 26;
    if (c >= '0' && c <= '9') return c - '0' + 52;
    if (c == '+') return 62;
    if (c == '/') return 63;
    return -1;
}

rv_status rv_decode_base64(const unsigned char *data, size_t len, rv_string *out)
{
    unsigned acc = 0;
    int bits = 0;
    size_t i;
    rv_status st;
    if (!data || !out) return RV_ERR_ARGUMENT;
    for (i = 0; i < len; i++) {
        int v;
        if (data[i] == '=' ) break;
        if (data[i] == '\r' || data[i] == '\n' || data[i] == ' ' || data[i] == '\t') continue;
        v = rv_b64_value(data[i]);
        if (v < 0) continue;
        acc = (acc << 6) | (unsigned)v;
        bits += 6;
        if (bits >= 8) {
            char b;
            bits -= 8;
            b = (char)((acc >> bits) & 0xffu);
            st = rv_string_append(out, &b, 1);
            if (st != RV_OK) return st;
        }
    }
    return RV_OK;
}

static int rv_hex_value(unsigned char c)
{
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'A' && c <= 'F') return c - 'A' + 10;
    if (c >= 'a' && c <= 'f') return c - 'a' + 10;
    return -1;
}

rv_status rv_decode_quoted_printable(const unsigned char *data, size_t len, rv_string *out)
{
    size_t i;
    rv_status st;
    if (!data || !out) return RV_ERR_ARGUMENT;
    for (i = 0; i < len; i++) {
        if (data[i] == '=' && i + 2 < len) {
            if (data[i + 1] == '\r' && data[i + 2] == '\n') { i += 2; continue; }
            if (data[i + 1] == '\n') { i += 1; continue; }
            if (i + 2 < len) {
                int a = rv_hex_value(data[i + 1]);
                int b = rv_hex_value(data[i + 2]);
                if (a >= 0 && b >= 0) {
                    char c = (char)((a << 4) | b);
                    st = rv_string_append(out, &c, 1);
                    if (st != RV_OK) return st;
                    i += 2;
                    continue;
                }
            }
        }
        st = rv_string_append(out, (const char *)data + i, 1);
        if (st != RV_OK) return st;
    }
    return RV_OK;
}

rv_status rv_decode_transfer(const char *encoding, const unsigned char *data, size_t len, rv_string *out)
{
    rv_encoding enc = rv_encoding_from_header(encoding);
    if (enc == RV_ENCODING_BASE64) return rv_decode_base64(data, len, out);
    if (enc == RV_ENCODING_QUOTED_PRINTABLE) return rv_decode_quoted_printable(data, len, out);
    return rv_string_append(out, (const char *)data, len);
}

rv_status rv_decode_rfc2047(const char *input, rv_string *out)
{
    const char *p;
    rv_status st;
    if (!input || !out) return RV_ERR_ARGUMENT;
    p = input;
    while (*p) {
        const char *start = strstr(p, "=?");
        if (!start) return rv_string_append_cstr(out, p);
        st = rv_string_append(out, p, (size_t)(start - p));
        if (st != RV_OK) return st;
        const char *q1 = strchr(start + 2, '?');
        const char *q2 = q1 ? strchr(q1 + 1, '?') : NULL;
        const char *end = q2 ? strstr(q2 + 1, "?=") : NULL;
        if (!q1 || !q2 || !end) {
            st = rv_string_append_cstr(out, start);
            if (st != RV_OK) return st;
            return RV_OK;
        }
        if (q1 + 1 < q2 && (q1[1] == 'B' || q1[1] == 'b')) {
            st = rv_decode_base64((const unsigned char *)q2 + 1, (size_t)(end - q2 - 1), out);
        } else if (q1 + 1 < q2 && (q1[1] == 'Q' || q1[1] == 'q')) {
            size_t i;
            for (i = 0; q2 + 1 + i < end; i++) {
                char c = q2[1 + i] == '_' ? ' ' : q2[1 + i];
                st = rv_string_append(out, &c, 1);
                if (st != RV_OK) return st;
            }
        } else {
            st = rv_string_append(out, q2 + 1, (size_t)(end - q2 - 1));
        }
        if (st != RV_OK) return st;
        p = end + 2;
    }
    return RV_OK;
}
