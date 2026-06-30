#ifndef RIVEMAIL_TYPES_H
#define RIVEMAIL_TYPES_H
#include <stddef.h>
#ifdef __cplusplus
extern "C" {
#endif

typedef enum rv_status {
    RV_OK = 0,
    RV_ERR_NOMEM = 1,
    RV_ERR_ARGUMENT = 2,
    RV_ERR_SYNTAX = 3,
    RV_ERR_LIMIT = 4,
    RV_ERR_IO = 5,
    RV_ERR_UNSUPPORTED = 6
} rv_status;

typedef enum rv_header_kind {
    RV_HEADER_UNKNOWN = 0,
    RV_HEADER_FROM,
    RV_HEADER_TO,
    RV_HEADER_CC,
    RV_HEADER_BCC,
    RV_HEADER_SUBJECT,
    RV_HEADER_DATE,
    RV_HEADER_MESSAGE_ID,
    RV_HEADER_CONTENT_TYPE,
    RV_HEADER_CONTENT_TRANSFER_ENCODING,
    RV_HEADER_MIME_VERSION,
    RV_HEADER_RECEIVED,
    RV_HEADER_DKIM_SIGNATURE,
    RV_HEADER_AUTHENTICATION_RESULTS,
    RV_HEADER_LIST_ID,
    RV_HEADER_LIST_UNSUBSCRIBE,
    RV_HEADER_IN_REPLY_TO,
    RV_HEADER_REFERENCES
} rv_header_kind;

typedef enum rv_encoding {
    RV_ENCODING_7BIT = 0,
    RV_ENCODING_8BIT,
    RV_ENCODING_BINARY,
    RV_ENCODING_BASE64,
    RV_ENCODING_QUOTED_PRINTABLE,
    RV_ENCODING_UNKNOWN
} rv_encoding;

typedef struct rv_span {
    const char *ptr;
    size_t len;
} rv_span;

typedef struct rv_string {
    char *data;
    size_t len;
    size_t cap;
} rv_string;

typedef struct rv_param {
    char *name;
    char *value;
    size_t source_offset;
    int quoted;
    int continuation;
} rv_param;

typedef struct rv_param_list {
    rv_param *items;
    size_t len;
    size_t cap;
} rv_param_list;

typedef struct rv_content_type {
    char *media_type;
    char *subtype;
    rv_param_list params;
    rv_param *tracked_boundary;
    size_t tracked_boundary_index;
    int has_boundary;
    int multipart;
    int malformed;
    int render_hint;
} rv_content_type;

typedef struct rv_header {
    char *name;
    char *value;
    char *raw;
    rv_header_kind kind;
    size_t line_no;
    size_t value_offset;
    int folded;
    rv_content_type content_type;
} rv_header;

typedef struct rv_header_list {
    rv_header *items;
    size_t len;
    size_t cap;
} rv_header_list;

typedef struct rv_part {
    rv_header_list headers;
    char *body;
    size_t body_len;
    size_t body_cap;
    struct rv_part *children;
    size_t child_count;
    size_t child_cap;
    char *boundary;
    char *preamble;
    size_t preamble_len;
    char *epilogue;
    size_t epilogue_len;
    char *epilogue_note_owned;
    rv_span epilogue_note;
    int is_message;
    int is_multipart;
} rv_part;

typedef struct rv_message {
    rv_header_list headers;
    rv_part root;
    char *body;
    size_t body_len;
    char *normalized_id;
    int saw_separator;
    int had_errors;
} rv_message;

typedef struct rv_options {
    size_t max_headers;
    size_t max_parts;
    size_t max_line_length;
    int allow_obsolete_folding;
    int normalize_line_endings;
    int preserve_raw_headers;
} rv_options;

typedef struct rv_diagnostic {
    rv_status code;
    char message[160];
    size_t line;
    size_t column;
} rv_diagnostic;

typedef struct rv_validation_result {
    rv_diagnostic *items;
    size_t len;
    size_t cap;
    int fatal_count;
    int warning_count;
} rv_validation_result;

typedef struct rv_mbox_entry {
    char *sender;
    char *date_text;
    rv_message message;
    size_t offset;
    size_t length;
} rv_mbox_entry;

typedef struct rv_mbox {
    rv_mbox_entry *entries;
    size_t len;
    size_t cap;
} rv_mbox;

#ifdef __cplusplus
}
#endif
#endif
