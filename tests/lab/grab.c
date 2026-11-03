/* Grab: save a rectangle of the frontmost screen as a PPM picture (P6, 24-bit
 * RGB), for lab pictures of the Workbench (Cradle's screenshot tool shows
 * the screen but does not keep it). Reads the screen through
 * cybergraphics.library's ReadPixelArray, so it works on RTG screens.
 *   Grab FILE [X Y W H]   (default: the whole screen)
 * A FILE ending in .png is written as PNG through GFOpenGPU's SDL2_image
 * (IMG_SavePNG): small enough to fetch from a lab. Fetch it with Chop.
 * MIT, Copyright (c) 2026 AmiCradle Team. */
#include <exec/types.h>
#include <exec/memory.h>
#include <intuition/intuitionbase.h>
#include <intuition/screens.h>
#include <proto/exec.h>
#include <proto/intuition.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <SDL.h>
#include <SDL_image.h>

struct IntuitionBase *IntuitionBase;
static struct Library *CyberGfxBase;

/* cybergraphics.library ReadPixelArray (LVO -120), RECTFMT_RGB (0) */
static ULONG read_pixel_array(APTR dst, UWORD dmod, struct RastPort *rp,
                              UWORD sx, UWORD sy, UWORD w, UWORD h)
{
    register ULONG d0 __asm("d0") = 0;
    register ULONG d1 __asm("d1") = 0;
    register ULONG d2 __asm("d2") = dmod;
    register ULONG d3 __asm("d3") = sx;
    register ULONG d4 __asm("d4") = sy;
    register ULONG d5 __asm("d5") = w;
    register ULONG d6 __asm("d6") = h;
    register ULONG d7 __asm("d7") = 0;
    register APTR a0 __asm("a0") = dst;
    register struct RastPort *a1 __asm("a1") = rp;
    register struct Library *a6 __asm("a6") = CyberGfxBase;
    __asm volatile ("jsr %%a6@(-120:W)"
                    : "+r"(d0), "+r"(d1), "+r"(a0), "+r"(a1)
                    : "r"(d2), "r"(d3), "r"(d4), "r"(d5), "r"(d6), "r"(d7), "r"(a6)
                    : "cc", "memory");
    return d0;
}

int main(int argc, char **argv)
{
    int rc = 20;
    if (argc != 2 && argc != 6) {
        printf("Grab FILE [X Y W H]\n");
        return 10;
    }
    IntuitionBase = (struct IntuitionBase *)OpenLibrary("intuition.library", 39);
    CyberGfxBase = OpenLibrary("cybergraphics.library", 40);
    if (IntuitionBase == NULL || CyberGfxBase == NULL) {
        printf("Grab: needs intuition.library 39 and cybergraphics.library 40\n");
        goto out;
    }
    ULONG lock = LockIBase(0);
    struct Screen *s = IntuitionBase->FirstScreen;
    int x = 0, y = 0, w = s->Width, h = s->Height;
    if (argc == 6) {
        x = atoi(argv[2]); y = atoi(argv[3]); w = atoi(argv[4]); h = atoi(argv[5]);
    }
    if (x < 0 || y < 0 || w <= 0 || h <= 0 || x + w > s->Width || y + h > s->Height) {
        UnlockIBase(lock);
        printf("Grab: the rectangle is not on the %dx%d screen\n", s->Width, s->Height);
        goto out;
    }
    UBYTE *buf = AllocVec((ULONG)w * h * 3, MEMF_ANY);
    if (buf == NULL) {
        UnlockIBase(lock);
        printf("Grab: no memory\n");
        goto out;
    }
    read_pixel_array(buf, (UWORD)(w * 3), &s->RastPort, (UWORD)x, (UWORD)y, (UWORD)w, (UWORD)h);
    int sw = s->Width, sh = s->Height;
    UnlockIBase(lock);
    size_t n = strlen(argv[1]);
    if (n > 4 && strcasecmp(argv[1] + n - 4, ".png") == 0) {
        SDL_Surface *surf = SDL_CreateRGBSurfaceWithFormatFrom(buf, w, h, 24, w * 3, SDL_PIXELFORMAT_RGB24);
        if (surf != NULL && IMG_SavePNG(surf, argv[1]) == 0) {
            printf("Grab: %dx%d from %d,%d of the %dx%d screen into %s (PNG)\n", w, h, x, y, sw, sh, argv[1]);
            rc = 0;
        } else {
            printf("Grab: cannot write %s: %s\n", argv[1], SDL_GetError());
        }
        if (surf != NULL) SDL_FreeSurface(surf);
        FreeVec(buf);
        goto out;
    }
    FILE *f = fopen(argv[1], "wb");
    if (f != NULL) {
        fprintf(f, "P6\n%d %d\n255\n", w, h);
        fwrite(buf, 3, (size_t)w * h, f);
        fclose(f);
        printf("Grab: %dx%d from %d,%d of the %dx%d screen into %s\n", w, h, x, y, sw, sh, argv[1]);
        rc = 0;
    } else {
        printf("Grab: cannot write %s\n", argv[1]);
    }
    FreeVec(buf);
out:
    if (CyberGfxBase) CloseLibrary(CyberGfxBase);
    if (IntuitionBase) CloseLibrary((struct Library *)IntuitionBase);
    return rc;
}
