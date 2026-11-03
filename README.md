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

## Installing

Make a drawer, for example `Work:Games/SDLPoP`, and put in it `prince`,
`SDLPoP.ini` and `COPYING` from the build, and the game's `data/` drawer
beside them. The program needs GFOpenGPU's `SDL2.module` and
`SDL2_image.module` in `LIBS:GFOpenGPU/`, AHI for sound, and a 68040 or
68060 with its FPU. From a Shell: `Stack 262144`, then `prince` in the
drawer.

## Status

**COMPILED**, not RELEASE (see Observed below and the release gates in
amicradle `docs/porting/MEASUREMENT.md`).

| Gate | State |
|---|---|
| Pinned source, reproducible build, receipts | done (open5, Cradle) |
| Start, info screen, title, level 1 shown, quit | seen once in a window (step 1 below) |
| Presentation, keyboard, pad, sound, saves, package, play test | not yet |

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
