/* untar: unpack a plain ustar archive into a drawer, for putting a game's
 * data on an Amiga that has no archiver (OS 3.2 ships none). From
 * amigapowermanga's runtime-data/untar.c.
 *   untar ARCHIVE DRAWER
 * Regular files and directories only; names are kept as they are, '/' being
 * the AmigaDOS separator too. Nothing outside DRAWER is written: a name that
 * starts with '/' or holds ':' or a ".." part is refused.
 * MIT, Copyright (c) 2026 AmiCradle Team. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

static unsigned long octal(const char *p, int n)
{
    unsigned long v = 0;
    while (n-- > 0 && *p == ' ')
        p++;
    while (n-- >= 0 && *p >= '0' && *p <= '7')
        v = v * 8 + (unsigned long)(*p++ - '0');
    return v;
}

static int safe(const char *name)
{
    const char *p = name;
    if (*name == '\0' || *name == '/' || strchr(name, ':') != NULL)
        return 0;
    while (p != NULL) {
        if (p[0] == '.' && p[1] == '.' && (p[2] == '/' || p[2] == '\0'))
            return 0;
        p = strchr(p, '/');
        if (p != NULL)
            p++;
    }
    return 1;
}

/* make every drawer on the way to PATH (PATH itself only when whole) */
static void drawers(char *path, int whole)
{
    char *p;
    for (p = path; *p; p++) {
        if (*p == '/' && p != path && p[-1] != ':') {
            *p = '\0';
            mkdir(path, 0777);
            *p = '/';
        }
    }
    if (whole)
        mkdir(path, 0777);
}

int main(int argc, char **argv)
{
    static char block[512], path[1024], buf[32768];
    FILE *in;
    unsigned long files = 0, bytes = 0;
    size_t base;
    if (argc != 3) {
        fprintf(stderr, "usage: untar ARCHIVE DRAWER\n");
        return 20;
    }
    if ((in = fopen(argv[1], "rb")) == NULL) {
        fprintf(stderr, "untar: cannot open %s\n", argv[1]);
        return 20;
    }
    strncpy(path, argv[2], sizeof(path) - 2);
    path[sizeof(path) - 2] = '\0';
    base = strlen(path);
    if (base > 0 && path[base - 1] != ':' && path[base - 1] != '/')
        path[base++] = '/';
    path[base] = '\0';
    drawers(path, 0);
    if (base > 0 && path[base - 1] == '/') {
        path[base - 1] = '\0';
        mkdir(path, 0777);
        path[base - 1] = '/';
    }
    while (fread(block, 1, 512, in) == 512) {
        char name[256 + 156];
        unsigned long size, left;
        char type = block[156];
        size_t n;
        FILE *out = NULL;
        if (block[0] == '\0')
            break;
        name[0] = '\0';
        if (memcmp(block + 257, "ustar", 5) == 0 && block[345] != '\0') {
            memcpy(name, block + 345, 155);
            name[155] = '\0';
            strcat(name, "/");
        }
        strncat(name, block, 100);
        size = octal(block + 124, 12);
        n = strlen(name);
        while (n > 0 && name[n - 1] == '/')
            name[--n] = '\0';
        if (n > 1 && name[0] == '.' && name[1] == '/')
            memmove(name, name + 2, n - 1);
        if (!safe(name) || base + strlen(name) >= sizeof(path)) {
            fprintf(stderr, "untar: refused %s\n", name);
            fclose(in);
            return 20;
        }
        strcpy(path + base, name);
        if (type == '5') {
            drawers(path, 1);
        } else if (type == '0' || type == '\0') {
            drawers(path, 0);
            if ((out = fopen(path, "wb")) == NULL) {
                fprintf(stderr, "untar: cannot write %s\n", path);
                fclose(in);
                return 20;
            }
            files++;
            bytes += size;
        }
        for (left = (size + 511) / 512 * 512; left > 0;) {
            size_t want = left < sizeof(buf) ? (size_t)left : sizeof(buf);
            size_t got = fread(buf, 1, want, in);
            if (got != want) {
                fprintf(stderr, "untar: %s is cut short\n", argv[1]);
                fclose(in);
                return 20;
            }
            if (out != NULL) {
                unsigned long used = (unsigned long)got;
                if (used > size)
                    used = size;
                if (used > 0 && fwrite(buf, 1, used, out) != used) {
                    fprintf(stderr, "untar: cannot write %s\n", path);
                    fclose(in);
                    return 20;
                }
                size -= used;
            }
            left -= got;
        }
        if (out != NULL)
            fclose(out);
    }
    fclose(in);
    printf("untar: %lu files, %lu bytes into %s\n", files, bytes, argv[2]);
    return 0;
}
