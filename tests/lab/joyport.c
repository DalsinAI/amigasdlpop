/* JoyPort: watch both game ports through lowlevel.library, as the game's
 * SDL2 joystick does, and print every change of state, for checking a stick
 * or CD32 pad (or Cradle's keyboard joystick) before blaming the game.
 *   JoyPort [SECS n]   (default 10)
 * Polls each port every 2 ms; prints the ticks (1/50 s), port, type and the
 * direction and button bits. MIT, Copyright (c) 2026 AmiCradle Team. */
#include <exec/types.h>
#include <dos/dos.h>
#include <libraries/lowlevel.h>
#include <proto/exec.h>
#include <proto/dos.h>
#include <proto/lowlevel.h>
#include <stdio.h>

struct Library *LowLevelBase;

static const char *type_name(ULONG s)
{
    switch (s & JP_TYPE_MASK) {
    case JP_TYPE_NOTAVAIL: return "none";
    case JP_TYPE_GAMECTLR: return "cd32pad";
    case JP_TYPE_MOUSE: return "mouse";
    case JP_TYPE_JOYSTK: return "joystick";
    default: return "unknown";
    }
}

int main(void)
{
    LONG args[1] = { 0 };
    struct RDArgs *rd = ReadArgs((STRPTR)"SECS/N", args, NULL);
    ULONG last[2] = { 0xffffffff, 0xffffffff };
    LONG ticks, end, changes = 0;
    int unit;
    if (!rd) { PrintFault(IoErr(), (STRPTR)"JoyPort"); return RETURN_FAIL; }
    if (!(LowLevelBase = OpenLibrary((CONST_STRPTR)"lowlevel.library", 40))) {
        printf("JoyPort: no lowlevel.library\n");
        FreeArgs(rd);
        return RETURN_FAIL;
    }
    {
        struct DateStamp ds;
        DateStamp(&ds);
        ticks = ds.ds_Minute * 3000 + ds.ds_Tick;
        end = ticks + (args[0] ? *(LONG *)args[0] : 10) * 50;
    }
    while (ticks < end && !(SetSignal(0, 0) & SIGBREAKF_CTRL_C)) {
        struct DateStamp ds;
        for (unit = 0; unit < 2; unit++) {
            ULONG s = ReadJoyPort(unit);
            if (s != last[unit]) {
                printf("%ld port %d %s%s%s%s%s%s%s%s%s%s%s\n", (long)ticks, unit, type_name(s),
                       s & JPF_JOY_UP ? " up" : "", s & JPF_JOY_DOWN ? " down" : "",
                       s & JPF_JOY_LEFT ? " left" : "", s & JPF_JOY_RIGHT ? " right" : "",
                       s & JPF_BUTTON_RED ? " red" : "", s & JPF_BUTTON_BLUE ? " blue" : "",
                       s & JPF_BUTTON_GREEN ? " green" : "", s & JPF_BUTTON_YELLOW ? " yellow" : "",
                       s & JPF_BUTTON_PLAY ? " play" : "",
                       (s & (JPF_BUTTON_FORWARD | JPF_BUTTON_REVERSE)) ? " shoulder" : "");
                last[unit] = s;
                changes++;
            }
        }
        Delay(1);
        DateStamp(&ds);
        ticks = ds.ds_Minute * 3000 + ds.ds_Tick;
    }
    printf("JoyPort: %ld changes\n", (long)changes);
    CloseLibrary(LowLevelBase);
    FreeArgs(rd);
    return 0;
}
