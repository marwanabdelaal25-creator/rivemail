#include "internal.h"

void rv_options_init(rv_options *opts)
{
    if (!opts) return;
    opts->max_headers = 1000;
    opts->max_parts = 200;
    opts->max_line_length = 998;
    opts->allow_obsolete_folding = 1;
    opts->normalize_line_endings = 1;
    opts->preserve_raw_headers = 1;
}

const char *rv_status_name(rv_status st)
{
    switch (st) {
    case RV_OK: return "ok";
    case RV_ERR_NOMEM: return "out of memory";
    case RV_ERR_ARGUMENT: return "invalid argument";
    case RV_ERR_SYNTAX: return "syntax error";
    case RV_ERR_LIMIT: return "configured limit reached";
    case RV_ERR_IO: return "input/output error";
    case RV_ERR_UNSUPPORTED: return "unsupported feature";
    default: return "unknown status";
    }
}

char *rv_strdup_len(const char *s, size_t n)
{
    char *p;
    if (!s && n) return NULL;
    p = (char *)malloc(n + 1);
    if (!p) return NULL;
    if (n) memcpy(p, s, n);
    p[n] = '\0';
    return p;
}

char *rv_strdup_cstr(const char *s)
{
    if (!s) return rv_strdup_len("", 0);
    return rv_strdup_len(s, strlen(s));
}

int rv_ascii_tolower(int c)
{
    if (c >= 'A' && c <= 'Z') return c + ('a' - 'A');
    return c;
}

int rv_ascii_casecmp(const char *a, const char *b)
{
    unsigned char ca, cb;
    if (a == b) return 0;
    if (!a) return -1;
    if (!b) return 1;
    while (*a && *b) {
        ca = (unsigned char)rv_ascii_tolower((unsigned char)*a++);
        cb = (unsigned char)rv_ascii_tolower((unsigned char)*b++);
        if (ca != cb) return (int)ca - (int)cb;
    }
    return (int)(unsigned char)*a - (int)(unsigned char)*b;
}

int rv_ascii_ncasecmp(const char *a, const char *b, size_t n)
{
    size_t i;
    if (a == b || n == 0) return 0;
    if (!a) return -1;
    if (!b) return 1;
    for (i = 0; i < n; i++) {
        unsigned char ca = (unsigned char)rv_ascii_tolower((unsigned char)a[i]);
        unsigned char cb = (unsigned char)rv_ascii_tolower((unsigned char)b[i]);
        if (ca != cb || ca == 0 || cb == 0) return (int)ca - (int)cb;
    }
    return 0;
}

size_t rv_trim_span(const char *s, size_t n, size_t *start_out)
{
    size_t b = 0;
    size_t e = n;
    while (b < e && (s[b] == ' ' || s[b] == '\t' || s[b] == '\r' || s[b] == '\n')) b++;
    while (e > b && (s[e - 1] == ' ' || s[e - 1] == '\t' || s[e - 1] == '\r' || s[e - 1] == '\n')) e--;
    if (start_out) *start_out = b;
    return e - b;
}

size_t rv_count_lines(const unsigned char *data, size_t len)
{
    size_t i;
    size_t lines = 1;
    if (!data || !len) return 0;
    for (i = 0; i < len; i++) {
        if (data[i] == '\n') lines++;
    }
    return lines;
}

uint32_t rv_hash_bytes(const unsigned char *data, size_t len)
{
    uint32_t h = 2166136261u;
    size_t i;
    for (i = 0; i < len; i++) {
        h ^= data[i];
        h *= 16777619u;
        h ^= h >> 13;
    }
    return h;
}

rv_status rv_string_reserve(rv_string *s, size_t need)
{
    char *p;
    size_t cap;
    if (!s) return RV_ERR_ARGUMENT;
    if (need <= s->cap) return RV_OK;
    cap = s->cap ? s->cap : 64;
    while (cap < need) {
        if (cap > ((size_t)-1) / 2) return RV_ERR_LIMIT;
        cap *= 2;
    }
    p = (char *)realloc(s->data, cap);
    if (!p) return RV_ERR_NOMEM;
    s->data = p;
    s->cap = cap;
    return RV_OK;
}

void rv_string_init(rv_string *s)
{
    if (!s) return;
    s->data = NULL;
    s->len = 0;
    s->cap = 0;
}

rv_status rv_string_append(rv_string *s, const char *data, size_t len)
{
    rv_status st;
    if (!s || (!data && len)) return RV_ERR_ARGUMENT;
    st = rv_string_reserve(s, s->len + len + 1);
    if (st != RV_OK) return st;
    if (len) memcpy(s->data + s->len, data, len);
    s->len += len;
    s->data[s->len] = '\0';
    return RV_OK;
}

rv_status rv_string_append_cstr(rv_string *s, const char *data)
{
    return rv_string_append(s, data ? data : "", data ? strlen(data) : 0);
}

void rv_string_free(rv_string *s)
{
    if (!s) return;
    free(s->data);
    s->data = NULL;
    s->len = 0;
    s->cap = 0;
}

int rv_header_name_equal(const char *a, const char *b)
{
    return rv_ascii_casecmp(a, b) == 0;
}

rv_header_kind rv_classify_header(const char *name)
{
    if (!name) return RV_HEADER_UNKNOWN;
    if (rv_header_name_equal(name, "from")) return RV_HEADER_FROM;
    if (rv_header_name_equal(name, "to")) return RV_HEADER_TO;
    if (rv_header_name_equal(name, "cc")) return RV_HEADER_CC;
    if (rv_header_name_equal(name, "bcc")) return RV_HEADER_BCC;
    if (rv_header_name_equal(name, "subject")) return RV_HEADER_SUBJECT;
    if (rv_header_name_equal(name, "date")) return RV_HEADER_DATE;
    if (rv_header_name_equal(name, "message-id")) return RV_HEADER_MESSAGE_ID;
    if (rv_header_name_equal(name, "content-type")) return RV_HEADER_CONTENT_TYPE;
    if (rv_header_name_equal(name, "content-transfer-encoding")) return RV_HEADER_CONTENT_TRANSFER_ENCODING;
    if (rv_header_name_equal(name, "mime-version")) return RV_HEADER_MIME_VERSION;
    if (rv_header_name_equal(name, "received")) return RV_HEADER_RECEIVED;
    if (rv_header_name_equal(name, "dkim-signature")) return RV_HEADER_DKIM_SIGNATURE;
    if (rv_header_name_equal(name, "authentication-results")) return RV_HEADER_AUTHENTICATION_RESULTS;
    if (rv_header_name_equal(name, "list-id")) return RV_HEADER_LIST_ID;
    if (rv_header_name_equal(name, "list-unsubscribe")) return RV_HEADER_LIST_UNSUBSCRIBE;
    if (rv_header_name_equal(name, "in-reply-to")) return RV_HEADER_IN_REPLY_TO;
    if (rv_header_name_equal(name, "references")) return RV_HEADER_REFERENCES;
    return RV_HEADER_UNKNOWN;
}

rv_encoding rv_encoding_from_header(const char *value)
{
    if (!value) return RV_ENCODING_UNKNOWN;
    while (*value == ' ' || *value == '\t') value++;
    if (rv_ascii_casecmp(value, "7bit") == 0) return RV_ENCODING_7BIT;
    if (rv_ascii_casecmp(value, "8bit") == 0) return RV_ENCODING_8BIT;
    if (rv_ascii_casecmp(value, "binary") == 0) return RV_ENCODING_BINARY;
    if (rv_ascii_casecmp(value, "base64") == 0) return RV_ENCODING_BASE64;
    if (rv_ascii_casecmp(value, "quoted-printable") == 0) return RV_ENCODING_QUOTED_PRINTABLE;
    return RV_ENCODING_UNKNOWN;
}

int rv_read_file(const char *path, unsigned char **data, size_t *len)
{
    FILE *f;
    long n;
    unsigned char *buf;
    if (!path || !data || !len) return 0;
    *data = NULL;
    *len = 0;
    f = fopen(path, "rb");
    if (!f) return 0;
    if (fseek(f, 0, SEEK_END) != 0) { fclose(f); return 0; }
    n = ftell(f);
    if (n < 0) { fclose(f); return 0; }
    if (fseek(f, 0, SEEK_SET) != 0) { fclose(f); return 0; }
    buf = (unsigned char *)malloc((size_t)n + 1);
    if (!buf) { fclose(f); return 0; }
    if ((size_t)n && fread(buf, 1, (size_t)n, f) != (size_t)n) { free(buf); fclose(f); return 0; }
    fclose(f);
    buf[n] = 0;
    *data = buf;
    *len = (size_t)n;
    return 1;
}
