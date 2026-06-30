#ifndef RIVEMAIL_INTERNAL_H
#define RIVEMAIL_INTERNAL_H
#include "rivemail/rivemail.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdint.h>
#include <errno.h>

#define RV_ARRAY_LEN(a) (sizeof(a) / sizeof((a)[0]))
#define RV_MIN(a,b) ((a) < (b) ? (a) : (b))
#define RV_MAX(a,b) ((a) > (b) ? (a) : (b))

char *rv_strdup_len(const char *s, size_t n);
char *rv_strdup_cstr(const char *s);
int rv_ascii_tolower(int c);
int rv_ascii_casecmp(const char *a, const char *b);
int rv_ascii_ncasecmp(const char *a, const char *b, size_t n);
size_t rv_trim_span(const char *s, size_t n, size_t *start_out);
size_t rv_count_lines(const unsigned char *data, size_t len);
rv_status rv_header_list_push(rv_header_list *list, rv_header *hdr);
void rv_header_list_free(rv_header_list *list);
void rv_header_free(rv_header *hdr);
rv_status rv_param_list_push(rv_param_list *list, rv_param *param);
void rv_param_list_free(rv_param_list *list);
rv_status rv_parse_header_block(const unsigned char *data, size_t len, const rv_options *opts, rv_header_list *headers, size_t *body_off);
rv_status rv_parse_content_type_value(const char *value, rv_content_type *ct);
void rv_content_type_free(rv_content_type *ct);
void rv_part_free(rv_part *part);
rv_header_kind rv_classify_header(const char *name);
rv_encoding rv_encoding_from_header(const char *value);
rv_status rv_render_header(const rv_header *hdr, rv_string *out);
rv_status rv_parse_part_body(rv_message *msg, const unsigned char *body, size_t len, const rv_options *opts);
rv_status rv_validate_headers(const rv_header_list *headers, rv_validation_result *result);
rv_status rv_validation_add(rv_validation_result *r, rv_status code, const char *msg, size_t line, size_t column, int fatal);
void rv_validation_free(rv_validation_result *r);
rv_status rv_normalize_headers(rv_header_list *headers);
rv_status rv_normalize_body(char **body, size_t *len);
int rv_looks_like_from_line(const char *s, size_t n);
rv_status rv_mbox_entry_push(rv_mbox *box, rv_mbox_entry *entry);
uint32_t rv_hash_bytes(const unsigned char *data, size_t len);
int rv_scan_token(const char *s, size_t n, size_t *used);
int rv_scan_quoted(const char *s, size_t n, size_t *used);
int rv_scan_comment(const char *s, size_t n, size_t *used);
rv_status rv_unfold_header_value(const char *value, rv_string *out);
rv_status rv_decode_quoted_printable(const unsigned char *data, size_t len, rv_string *out);
rv_status rv_decode_base64(const unsigned char *data, size_t len, rv_string *out);
rv_status rv_render_folded_line(const char *name, const char *value, rv_string *out);
rv_status rv_address_parse_mailboxes(const char *input, rv_string *out);
int rv_read_file(const char *path, unsigned char **data, size_t *len);
int rv_mimeflow_public_score(const char *s, size_t n);

#endif
