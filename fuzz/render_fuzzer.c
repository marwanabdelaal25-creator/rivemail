#include "rivemail/rivemail.h"
#include <stdint.h>
#include <stddef.h>

int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    rv_message msg;
    rv_string out;
    if (size > 32768) return 0;
    rv_string_init(&out);
    if (rv_message_parse(data, size, NULL, &msg) == RV_OK) {
        (void)rv_message_render(&msg, &out);
        rv_string_free(&out);
        rv_message_free(&msg);
    }
    return 0;
}
