# SDLPoP for AmigaOS 3.x / AmiCradle

SDLPoP is the open-source port of Jordan Mechner's Prince of Persia (1989),
made from the disassembly of the DOS game, with SDL 2 for display, sound and
input. GPL-3.0-or-later (upstream `COPYING`); `src/opl3.c` and
`src/stb_vorbis.c` carry their own licences. This repository holds the
AmiCradle port around upstream
[NagyD/SDLPoP](https://github.com/NagyD/SDLPoP) at
`3c5add5fb7f83d4ceb542823ab66d00146c4271b` (SDLPoP 1.24 RC, 24 December
2025): the patches, the build, the lab tools and the package. Target:
**AC090 / AmigaOS 3.x / 68040 + FPU**, libnix, SDL 2 through GFOpenGPU's
`SDL2.module` and `SDL2_image.module`.

## Game data

SDLPoP needs the original game's data (`data/`: the DAT files, the sprites,
the levels and the sounds). Upstream's repository carries it, but it is the
1989 game's, not SDLPoP's, and `STANDALONE.md` and `gameports/catalog.json`
say the commercial Prince of Persia data is not bundled: the user supplies
it. So this repository does not commit it and the package leaves it out.
`runtime-data/build-amiga.sh` stages upstream's `data/` at the pinned commit
for lab tests only.

## Building

The port never edits upstream's tree: `build-amiga.sh` fetches the pinned
commit (or takes `SDLPOP_SOURCE`, a local checkout holding it), applies
`patches/` to an exported copy with no fuzz, and compiles upstream's own
source list (`src/Makefile`) with upstream's flags (`-std=c99 -O2 -Wall`)
for the 68040 and FPU, linking the stove's `libSDL2.a` and `libSDL2_image.a`
(GFOpenGPU's link stubs).

Through Cradle (the build the lab tests use): `stove_build` with the stove
`os32-gcc16`, target `.` for the program, `runtime-data` for the lab's game
data and `tests/lab` for the lab tools. From a checkout, with a staged
stove:

```sh
STOVE=/path/to/os32-gcc16 OUT=build-amiga sh build-amiga.sh
```

`scripts/verify_gameport_stove.py` takes OpenAmigaGCC 16.2.0-open3 (archive
SHA-256 `9fcc136ab59e3bb5b91d2764d0bf290b709d72d7380be4ce0cdf005edf8eff21`),
16.2.0-open4 (`495319a2ffd803278f8827cad70074029c7aa9ecbe05d037e8646ba43ab94ab2`)
or GFOpenAmigaGCC 16.2.0-open5 (Cradle's `os32-gcc16` since the evening of
2 November 2026; no archive is published, so it is taken by `STOVE.json`'s
release and the compiler's own `--version`, and the receipt records the
SHA-256 of the compiler's own programs) and refuses any other.
`build-amigaos3.sh SOURCE OUT`, the earlier entry point, runs the same
build. The output is the drawer `SDLPoP/` (`prince`, upstream's
`SDLPoP.ini`, `COPYING`) and `receipt.json`, which names the compiler, the
patches' and the recipe's digests, the SDK inputs and the executable's
SHA-256; the receipts of the builds recorded here are in `receipts/`.

The patches, all AmigaOS only (`__amigaos__`):

- `0001-amigaos3-opengpu.patch`: `alloca` from the compiler, the current
  directory `PROGDIR:`, `logf` for the missing `log2f`.
- `0002-amiga-paths.patch`: files beside the program; SDLPoP.cfg,
  QUICKSAVE.SAV, PRINCE.HOF and screenshots in `PROGDIR:User`; the whole
  `SDLPoP.ini` read with libnix's `fscanf`; a `$VER` string.
- `0003-amiga-presentation.patch`: a 320x200 screen of the game's own by
  default (SDL's real full screen), `window` for a window, the renderer in
  the log, `fps` logging the frame rate.

## Installing

Make a drawer, for example `Work:Games/SDLPoP`, and put in it `prince`,
`SDLPoP.ini` and `COPYING` from the build, and the game's `data/` drawer
beside them. The program needs GFOpenGPU's `SDL2.module` and
`SDL2_image.module` in `LIBS:GFOpenGPU/`, AHI for sound, and a 68040 or
68060 with its FPU. From a Shell: `Stack 262144`, then `prince` in the
drawer.

## Running

`prince` with no options opens a 320x200 screen of its own, the game's own
frame, unscaled (SDL's real full screen through GFOpenGPU; upstream's
default is a 640x400 window, and its full screen is the desktop's size,
every frame scaled to it). `SDLPoP.ini`'s `start_fullscreen = false`, or
`prince window`, gives upstream's 640x400 window on the Workbench,
resizable; Alt+Return switches between the two, and from the window it
makes a screen of the window's size (640x400: each pixel doubled). `prince
fps` logs every second the game frames played, the screen updates and what
an update cost (`fps game 12.0, screen 12.0, update 2.07 ms (max 3.28) at
147.602 s`).

## Status

**COMPILED**, not RELEASE (see Observed below and the release gates in
amicradle `docs/porting/MEASUREMENT.md`).

| Gate | State |
|---|---|
| Pinned source, reproducible build, receipts | done (open5, Cradle) |
| Start, info screen, title, intro, demo level, quit | done |
| Whole 320x200 frame on a screen of its own, frame rate measured | done (12.0 game frames a second, as the game's own timer gives) |
| Keyboard, pad, sound, saves, package, play test | not yet |

## Observed

### Build, 3 November 2026

Cradle `stove_build`, stove `os32-gcc16` (GFOpenAmigaGCC 16.2.0-open5),
target `.`:

| Commit | Job | Executable SHA-256 | Bytes |
|---|---|---|---|
| `3e9b058` (0001 only) | `j20261103T013956-4a8693` | `976f5f531608eab1f334a03d8015897552ce2e6fa1a781bb223cfccac7cdb2c8` | 593,424 |
| `65372f8` (0001, 0002) | `j20261103T015531-934dcf` | `4dd2efb6d181800f999a5db8ef7ae557224f63c10a1aeca28b78feb1aa9e32d0` | 593,352 |

The compiler's warnings are upstream's own (two `isspace` subscripts, one
always-true comparison), none in the patches. A local build with open3
gives the same warnings. Lab data (target `runtime-data`, job
`j20261103T014106-460abd`): 1,034 files, archive `510b70e5...`, `untar`
`2de6090b...`; lab tools (target `tests/lab`, job
`j20261103T014129-972d39`).

In `lab:sdlpop`, a copy of Instance-32 (AC090 JIT, 68040 + FPU, RTG
1920x1080, AHI, muted), with `SDL2.module` built by Cradle from gfopenrtg
main at `77fb11f` (job `j20261103T013628-da91df`, SHA-256
`eec57f96aa23fc26124b11e6ba0dbe25eb330d5c48ef4cb73f5d0475dd53a60b`,
1,002,284 bytes) in both `LIBS:OpenGPU/` and `LIBS:GFOpenGPU/`, and the
Instance's `SDL2_image.module` copied into `LIBS:GFOpenGPU/` (the program
stops without it there: `SDL2_image: this program needs
LIBS:GFOpenGPU/SDL2_image.module`); the game in `Work:Games/SDLPoP` with
its data unpacked by `untar`:

- `3e9b058`: upstream's 640x400 window on the Workbench (`OS3_FB: window
  surface in video RAM (640x400, 2560 bytes a row)`), the information
  screen, the title and Level 1, colours right; Ctrl+Q ended it. It also
  printed `short read from SDLPoP.ini!?`: libnix's `fscanf` gives EOF where
  glibc gives 0 at the end of the file, and upstream stopped reading the
  settings there.
- `c5705df` (0002's first form): every Ctrl+Q hung the whole lab (the CPU
  in a ROM loop with interrupts off). libnix frees its own `argv` strings
  at exit (`FreeVec` in `__exitcommandline`), and the patch had put a
  static string in `argv[0]`. `65372f8` gives the game a copy of the list
  instead.
- `65372f8`: no message about the settings, and `start_minutes_left = 42`
  added at the end of `SDLPoP.ini` under `[CustomGameplay]` gave "42
  MINUTES LEFT" in Level 1, so the whole file is read; Ctrl+Q from the
  information screen and from Level 1 ended it cleanly (three runs);
  `PROGDIR:User` was made at the start.

### Presentation, 3 November 2026

Commit `6eb1593` (`patches/0003-amiga-presentation.patch`, on top of 0001
and 0002): on AmigaOS the game's own 320x200 frame on a screen of its own by
default, `window` for upstream's window, the renderer and the output size
in the log, and `fps` logging each second. Executable (open5, Cradle job
`j20261103T020043-d57fff`): SHA-256
`f4613aaf92ee0e2652ef9ceb0649bcf126e1bba0e9a661096d80464fd0d267db`
(595,132 bytes; `receipts/2-presentation-open5-stove_build.json`).

In `lab:sdlpop` as for the build, `prince fps`, left to run from the
information screen through the title and the intro into the demo level:

- `OS3_FB: window surface in video RAM (320x200, 1280 bytes a row)` and
  `SDL renderer "opengpu" (flags 0xa), full screen 320x200, output
  320x200`: the RTG head switched to 320x200 and every screen filled it,
  frame whole; SDL picked GFOpenGPU's accelerated renderer (`opengpu`,
  render targets supported), so upstream's texture path runs as it is.
- The game's logic runs on its own timer: in the demo level **12.0 game
  frames a second** in all six readings (11.9 at the lowest), which is
  what the original's 60 Hz timer and its 5-tick frame give, and the
  screen was updated once a game frame. An update (upstream's surface to
  texture to screen) took **1.8 to 3.1 ms on average**, 11.6 ms at most;
  at 12 frames a second that is about 3 per cent of the time.
- In the title and the intro, which redraw on their own timers, 26 to 53
  updates a second at 1.6 to 6.3 ms each on average; the longest single
  update in the 150 s was 215 ms, once, while the intro's music and
  pictures were being loaded.
- `prince window`: `window 640x400, output 640x400`, the information screen
  and the title in a window on the Workbench, each pixel doubled;
  Alt+Return gave a 640x400 screen of its own and Alt+Return again the
  window. Ctrl+Q ended every run cleanly, back to the Workbench.

![The information screen on its own 320x200 screen](docs/observed/step2-info-320x200.png)
![The title](docs/observed/step2-title-presents.png)
![The intro](docs/observed/step2-cutscene-princess.png)
