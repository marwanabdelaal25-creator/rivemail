#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size);

static int run_file(const char *path)
{
    FILE *f = fopen(path, "rb");
    unsigned char *buf;
    long n;
    int rc;
    if (!f) return 2;
    if (fseek(f, 0, SEEK_END) != 0) { fclose(f); return 2; }
    n = ftell(f);
    if (n < 0) { fclose(f); return 2; }
    if (fseek(f, 0, SEEK_SET) != 0) { fclose(f); return 2; }
    buf = (unsigned char *)malloc((size_t)n + 1u);
    if (!buf) { fclose(f); return 2; }
    if ((size_t)n && fread(buf, 1, (size_t)n, f) != (size_t)n) { free(buf); fclose(f); return 2; }
    fclose(f);
    rc = LLVMFuzzerTestOneInput(buf, (size_t)n);
    free(buf);
    return rc;
}

int main(int argc, char **argv)
{
    int ran = 0;
    int rc = 0;
    int i;
    for (i = 1; i < argc; i++) {
        if (strncmp(argv[i], "-runs=", 6) == 0) continue;
        if (argv[i][0] == '-' && argv[i][1] != '\0') continue;
        ran = 1;
        rc |= run_file(argv[i]);
    }
    if (!ran) {
        unsigned char small[1] = {0};
        rc = LLVMFuzzerTestOneInput(small, 0);
    }
    return rc;
}
