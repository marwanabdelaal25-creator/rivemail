#include "internal.h"

static void rv_cli_usage(FILE *out)
{
    fprintf(out, "usage: rivemail [render|validate|headers|mbox] FILE\n");
}

static int rv_cli_render(const unsigned char *data, size_t len)
{
    rv_message msg;
    rv_string out;
    rv_status st;
    rv_string_init(&out);
    st = rv_message_parse(data, len, NULL, &msg);
    if (st != RV_OK) { fprintf(stderr, "parse: %s\n", rv_status_name(st)); return 2; }
    st = rv_message_render(&msg, &out);
    if (st == RV_OK && out.data) fwrite(out.data, 1, out.len, stdout);
    rv_message_free(&msg);
    rv_string_free(&out);
    return st == RV_OK ? 0 : 2;
}

static int rv_cli_validate(const unsigned char *data, size_t len)
{
    rv_message msg;
    rv_validation_result vr;
    size_t i;
    rv_status st;
    st = rv_message_parse(data, len, NULL, &msg);
    if (st != RV_OK) { fprintf(stderr, "parse: %s\n", rv_status_name(st)); return 2; }
    st = rv_message_validate(&msg, &vr);
    if (st != RV_OK) { rv_message_free(&msg); return 2; }
    for (i = 0; i < vr.len; i++) {
        fprintf(stdout, "%s:%zu:%zu: %s\n", rv_status_name(vr.items[i].code), vr.items[i].line, vr.items[i].column, vr.items[i].message);
    }
    rv_validation_free(&vr);
    rv_message_free(&msg);
    return 0;
}

static int rv_cli_headers(const unsigned char *data, size_t len)
{
    rv_message msg;
    size_t i;
    rv_status st;
    st = rv_message_parse(data, len, NULL, &msg);
    if (st != RV_OK) { fprintf(stderr, "parse: %s\n", rv_status_name(st)); return 2; }
    for (i = 0; i < msg.headers.len; i++) {
        printf("%s: %s\n", msg.headers.items[i].name, msg.headers.items[i].value);
    }
    rv_message_free(&msg);
    return 0;
}

static int rv_cli_mbox(const unsigned char *data, size_t len)
{
    rv_mbox box;
    size_t i;
    rv_status st;
    st = rv_mbox_parse(data, len, NULL, &box);
    if (st != RV_OK) { fprintf(stderr, "mbox: %s\n", rv_status_name(st)); return 2; }
    for (i = 0; i < box.len; i++) {
        printf("entry %zu sender=%s bytes=%zu\n", i + 1, box.entries[i].sender ? box.entries[i].sender : "", box.entries[i].length);
    }
    rv_mbox_free(&box);
    return 0;
}

int rv_cli_main(int argc, char **argv)
{
    unsigned char *data = NULL;
    size_t len = 0;
    int rc;
    if (argc != 3) { rv_cli_usage(stderr); return 2; }
    if (!rv_read_file(argv[2], &data, &len)) { perror(argv[2]); return 2; }
    if (strcmp(argv[1], "render") == 0) rc = rv_cli_render(data, len);
    else if (strcmp(argv[1], "validate") == 0) rc = rv_cli_validate(data, len);
    else if (strcmp(argv[1], "headers") == 0) rc = rv_cli_headers(data, len);
    else if (strcmp(argv[1], "mbox") == 0) rc = rv_cli_mbox(data, len);
    else { rv_cli_usage(stderr); rc = 2; }
    free(data);
    return rc;
}

#ifdef RIVEMAIL_STANDALONE
int main(int argc, char **argv)
{
    return rv_cli_main(argc, argv);
}
#endif
