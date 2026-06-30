#ifndef RIVEMAIL_H
#define RIVEMAIL_H
#include "rivemail/export.h"
#include "rivemail/types.h"
#include <stddef.h>
#ifdef __cplusplus
extern "C" {
#endif

RIVEMAIL_API void rv_options_init(rv_options *opts);
RIVEMAIL_API const char *rv_status_name(rv_status st);
RIVEMAIL_API rv_status rv_message_parse(const unsigned char *data, size_t len, const rv_options *opts, rv_message *out);
RIVEMAIL_API void rv_message_init(rv_message *msg);
RIVEMAIL_API void rv_message_free(rv_message *msg);
RIVEMAIL_API rv_status rv_message_render(const rv_message *msg, rv_string *out);
RIVEMAIL_API rv_status rv_message_normalize(rv_message *msg, const rv_options *opts);
RIVEMAIL_API rv_status rv_message_validate(const rv_message *msg, rv_validation_result *result);
RIVEMAIL_API void rv_validation_free(rv_validation_result *result);
RIVEMAIL_API rv_header *rv_message_find_header(rv_message *msg, const char *name);
RIVEMAIL_API const rv_header *rv_message_find_header_const(const rv_message *msg, const char *name);
RIVEMAIL_API rv_status rv_mbox_parse(const unsigned char *data, size_t len, const rv_options *opts, rv_mbox *out);
RIVEMAIL_API void rv_mbox_free(rv_mbox *box);
RIVEMAIL_API void rv_string_init(rv_string *s);
RIVEMAIL_API rv_status rv_string_reserve(rv_string *s, size_t need);
RIVEMAIL_API rv_status rv_string_append(rv_string *s, const char *data, size_t len);
RIVEMAIL_API rv_status rv_string_append_cstr(rv_string *s, const char *data);
RIVEMAIL_API void rv_string_free(rv_string *s);
RIVEMAIL_API rv_status rv_decode_rfc2047(const char *input, rv_string *out);
RIVEMAIL_API rv_status rv_decode_transfer(const char *encoding, const unsigned char *data, size_t len, rv_string *out);
RIVEMAIL_API rv_status rv_address_list_normalize(const char *input, rv_string *out);
RIVEMAIL_API int rv_header_name_equal(const char *a, const char *b);
RIVEMAIL_API int rv_is_boundary_safe(const char *boundary);
RIVEMAIL_API int rv_cli_main(int argc, char **argv);

#ifdef __cplusplus
}
#endif
#endif
