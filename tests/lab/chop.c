/* Chop: cut a file into pieces of at most SIZE bytes, FILE.0, FILE.1 ...,
 * for fetching a screenshot from a lab with a tool that returns only small
 * files. Join them again with cat FILE.0 FILE.1 ... on the PC. With a third
 * argument (PAD) the last piece is padded with zeros to SIZE as well (the
 * file's length is printed, to cut the padding off again).
 *   Chop FILE SIZE [PAD]  (SIZE default 40000)
 * MIT, Copyright (c) 2026 AmiCradle Team. */
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv)
{
    static char buf[65536];
    long size = argc > 2 ? atol(argv[2]) : 40000;
    int pad = argc > 3;
    unsigned long total = 0;
    FILE *in;
    int n = 0;
    if (argc < 2 || size < 1 || size > (long)sizeof(buf)) {
        fprintf(stderr, "usage: Chop FILE [SIZE]\n");
        return 20;
    }
    if (!(in = fopen(argv[1], "rb"))) {
        fprintf(stderr, "Chop: can't open %s\n", argv[1]);
        return 20;
    }
    for (;;) {
        size_t got = fread(buf, 1, (size_t)size, in);
        char name[512];
        FILE *out;
        if (got == 0)
            break;
        total += got;
        if (pad && got < (size_t)size) {
            size_t i;
            for (i = got; i < (size_t)size; ++i)
                buf[i] = 0;
            got = (size_t)size;
        }
        snprintf(name, sizeof(name), "%s.%d", argv[1], n++);
        if (!(out = fopen(name, "wb")) || fwrite(buf, 1, got, out) != got) {
            fprintf(stderr, "Chop: can't write %s\n", name);
            return 20;
        }
        fclose(out);
        printf("%s %lu\n", name, (unsigned long)got);
    }
    fclose(in);
    printf("%s %lu bytes in %d pieces\n", argv[1], total, n);
    return 0;
}
