#include "rivemail/rivemail.h"
#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    rv_message msg;
    rv_validation_result vr;
    if (size > 32768) return 0;
    if (rv_message_parse(data, size, NULL, &msg) == RV_OK) {
        (void)rv_message_validate(&msg, &vr);
        rv_validation_free(&vr);
        rv_message_free(&msg);
    }
    return 0;
}
