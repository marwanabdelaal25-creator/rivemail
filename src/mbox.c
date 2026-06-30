#include "internal.h"

int rv_looks_like_from_line(const char *s, size_t n)
{
    size_t i;
    int spaces = 0;
    if (!s || n < 6) return 0;
    if (memcmp(s, "From ", 5) != 0) return 0;
    for (i = 5; i < n; i++) {
        if (s[i] == ' ') spaces++;
        if (s[i] == '\r' || s[i] == '\n') break;
    }
    return spaces >= 2;
}

static size_t rv_next_line(const unsigned char *data, size_t len, size_t pos)
{
    while (pos < len && data[pos] != '\n') pos++;
    if (pos < len) pos++;
    return pos;
}

static rv_status rv_parse_mbox_entry_header(const unsigned char *line, size_t len, rv_mbox_entry *entry)
{
    size_t i = 5;
    size_t sender_start;
    size_t sender_len;
    if (!entry || len < 5) return RV_ERR_ARGUMENT;
    while (i < len && line[i] == ' ') i++;
    sender_start = i;
    while (i < len && line[i] != ' ' && line[i] != '\r' && line[i] != '\n') i++;
    sender_len = i - sender_start;
    entry->sender = rv_strdup_len((const char *)line + sender_start, sender_len);
    while (i < len && line[i] == ' ') i++;
    entry->date_text = rv_strdup_len((const char *)line + i, len - i);
    if (!entry->sender || !entry->date_text) return RV_ERR_NOMEM;
    return RV_OK;
}

rv_status rv_mbox_parse(const unsigned char *data, size_t len, const rv_options *opts, rv_mbox *out)
{
    size_t pos = 0;
    rv_status st;
    if (!data || !out) return RV_ERR_ARGUMENT;
    memset(out, 0, sizeof(*out));
    while (pos < len) {
        size_t line_end = pos;
        size_t content_start;
        size_t next;
        rv_mbox_entry entry;
        memset(&entry, 0, sizeof(entry));
        while (line_end < len && data[line_end] != '\r' && data[line_end] != '\n') line_end++;
        if (!rv_looks_like_from_line((const char *)data + pos, line_end - pos)) {
            pos = rv_next_line(data, len, pos);
            continue;
        }
        st = rv_parse_mbox_entry_header(data + pos, line_end - pos, &entry);
        if (st != RV_OK) { rv_mbox_free(out); return st; }
        content_start = rv_next_line(data, len, pos);
        next = content_start;
        while (next < len) {
            size_t e = next;
            while (e < len && data[e] != '\r' && data[e] != '\n') e++;
            if (rv_looks_like_from_line((const char *)data + next, e - next)) break;
            next = rv_next_line(data, len, next);
        }
        entry.offset = content_start;
        entry.length = next - content_start;
        st = rv_message_parse(data + content_start, entry.length, opts, &entry.message);
        if (st != RV_OK) { free(entry.sender); free(entry.date_text); rv_mbox_free(out); return st; }
        st = rv_mbox_entry_push(out, &entry);
        if (st != RV_OK) { free(entry.sender); free(entry.date_text); rv_message_free(&entry.message); rv_mbox_free(out); return st; }
        pos = next;
    }
    return RV_OK;
}
