#include "rivemail/rivemail.h"
#include <stdint.h>
#include <stddef.h>

int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    rv_message msg;
    rv_string rendered;
    rv_validation_result vr;
    rv_options opts;
    if (size > 65536) return 0;
    rv_options_init(&opts);
    opts.max_headers = 2000;
    rv_string_init(&rendered);
    if (rv_message_parse(data, size, &opts, &msg) == RV_OK) {
        (void)rv_message_validate(&msg, &vr);
        rv_validation_free(&vr);
        (void)rv_message_render(&msg, &rendered);
        rv_string_free(&rendered);
        rv_message_free(&msg);
    }
    return 0;
}
