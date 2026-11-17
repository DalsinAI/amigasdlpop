#!/bin/sh
# SDLPoP for AmigaOS 3.x / AmiCradle (68040 + FPU, SDL 2 through GFOpenGPU),
# built with the os32-gcc16 stove: OpenAmigaGCC 16.2.0-open3 or open4, known
# by their archives' SHA-256, or GFOpenAmigaGCC 16.2.0-open5 (Cradle's
# os32-gcc16 stove since the evening of 2 November 2026), which has no
# published archive: scripts/verify_gameport_stove.py takes it by STOVE.json's
# release and the compiler's own --version, and the receipt records the
# SHA-256 of the compiler's own programs instead. This is the build Cradle's
# stove_build runs (STOVE= and OUT= set, target .).
#   STOVE          the stove (default ~/AmiCradle/stoves/os32-gcc16, then
#                  ~/AmigaChrome/stoves/os32-gcc16)
#   OUT            the output directory (default build-amiga)
#   SDLPOP_SOURCE  a local upstream checkout holding the pinned commit;
#                  without it the pinned commit is fetched from upstream
#   JOBS           parallel compiles (default 2)
# The pinned commit is gameports/catalog.json's; patches/ is applied to an
# exported copy with no fuzz. Upstream's own source list (src/Makefile, OBJ)
# is compiled with the stove's compiler and linked with the stove's libSDL2.a
# and libSDL2_image.a (GFOpenGPU's link stubs).
# Outputs: SDLPoP/prince (the program), SDLPoP/SDLPoP.ini, SDLPoP/COPYING,
# receipt.json, build.log. The Prince of Persia game data (upstream's data/)
# is not built or shipped here: runtime-data/build-amiga.sh stages it for a
# lab only.
# MIT, Copyright (c) 2026 AmiCradle Team.
set -eu

HERE=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
if [ -z "${STOVE:-}" ]; then
    STOVE="$HOME/AmiCradle/stoves/os32-gcc16"
    [ -d "$STOVE" ] || STOVE="$HOME/AmigaChrome/stoves/os32-gcc16"
fi
OUT=${OUT:-"$HERE/build-amiga"}
JOBS=${JOBS:-2}
P="$STOVE/prefix"
SDK="$P/m68k-amigaos"
CC="$P/bin/m68k-amigaos-gcc"

# the compiler must be one of the three (prints "RELEASE ARCHIVE-SHA256",
# ARCHIVE-SHA256 "unpublished" for open5)
COMPILER=$(python3 "$HERE/scripts/verify_gameport_stove.py" --stove "$STOVE")
RELEASE=${COMPILER% *}
ARCHIVE=${COMPILER#* }
"$CC" --version | head -n 1
"$CC" --version | head -n 1 | grep -q "$RELEASE" || { echo "$CC is not $RELEASE"; exit 2; }

# SDL 2 and SDL2_image from the stove's SDK (GFOpenGPU's link stubs)
for f in include/SDL2/SDL.h include/SDL2/SDL_image.h lib/libSDL2.a lib/libSDL2_image.a; do
    [ -f "$SDK/$f" ] || { echo "the stove has no $f"; exit 2; }
done

PIN=$(python3 -c 'import json,sys; print(json.load(open(sys.argv[1]))["ports"]["sdlpop"]["source"]["commit"])' "$HERE/gameports/catalog.json")
URL=$(python3 -c 'import json,sys; print(json.load(open(sys.argv[1]))["ports"]["sdlpop"]["source"]["url"])' "$HERE/gameports/catalog.json")
REPO_REV=$(git -C "$HERE" rev-parse HEAD 2>/dev/null || echo unknown)

WORK=$(mktemp -d "${TMPDIR:-/tmp}/sdlpop-build.XXXXXX")
trap 'rm -rf "$WORK"' EXIT INT TERM
mkdir -p "$OUT"

if [ -n "${SDLPOP_SOURCE:-}" ]; then
    SRC=$SDLPOP_SOURCE
else
    SRC="$WORK/upstream"
    git init -q "$SRC"
    git -C "$SRC" fetch -q --depth 1 "$URL" "$PIN"
fi
git -C "$SRC" cat-file -e "$PIN^{commit}"
EPOCH=$(git -C "$SRC" show -s --format=%ct "$PIN")

# the engine's source, its settings file and its licence; not data/
mkdir -p "$WORK/src"
git -C "$SRC" archive "$PIN" src SDLPoP.ini COPYING | tar -xf - -C "$WORK/src"
for patch in "$HERE"/patches/*.patch; do
    [ -f "$patch" ] || continue
    echo "patch: $(basename "$patch")"
    patch --batch --forward --fuzz=0 -p1 -d "$WORK/src" < "$patch"
done

# upstream's source list, as src/Makefile has it
OBJS=$(sed -n 's/^OBJ *= *//p' "$WORK/src/src/Makefile")
[ "$(printf '%s\n' $OBJS | wc -l)" -ge 20 ] || { echo "could not read the source list"; exit 2; }

# upstream's flags (src/Makefile: -std=c99 -O2 -Wall -D_GNU_SOURCE=1), the
# target's, and -fno-delete-null-pointer-checks: address 0 is memory on an
# Amiga
FLAGS="-noixemul -m68040 -msoft-float -pthread -fno-delete-null-pointer-checks
 -std=c99 -O2 -Wall -D_GNU_SOURCE=1 -ffile-prefix-map=$WORK/src=.
 -I$SDK/include/SDL2"
LIBS="-lSDL2_image -lSDL2 -lGL -lm -pthread"

(
    cd "$WORK/src/src"
    export SOURCE_DATE_EPOCH="$EPOCH"
    printf '%s\n' $OBJS | xargs -P "$JOBS" -I{} sh -c '
        c=$(basename "{}" .o).c
        echo "$0" $1 -c "$c" -o "{}"
        "$0" $1 -c "$c" -o "{}"' "$CC" "$FLAGS"
    # shellcheck disable=SC2086
    echo "$CC" -noixemul -m68040 -msoft-float -pthread -o prince $OBJS -L"$SDK/lib" $LIBS
    # shellcheck disable=SC2086
    "$CC" -noixemul -m68040 -msoft-float -pthread -o prince $OBJS -L"$SDK/lib" $LIBS
) 2>&1 | tee "$OUT/build.log"

EXE="$WORK/src/src/prince"
[ -f "$EXE" ] || { echo "no executable: see build.log"; exit 2; }
[ "$(od -An -tx1 -N4 "$EXE" | tr -d ' \n')" = "000003f3" ] || { echo "not an Amiga HUNK executable"; exit 2; }
# One binary for every 68040 and 68060: -m68040 -msoft-float throughout, no
# FPU instruction in the objects, the stove's fpuauto linked in
# (package/soft-float.sh).
sh "$HERE/package/soft-float.sh" "$P" "$EXE" --log "$OUT/build.log" "$WORK/src/src" > "$WORK/soft-float.txt" ||
    { cat "$WORK/soft-float.txt"; exit 1; }
cat "$WORK/soft-float.txt"

# the drawer: the program, upstream's settings file and the licence (the
# game data goes beside them: data/, from the user)
D="$OUT/SDLPoP"
mkdir -p "$D"
cp "$EXE" "$D/prince"
cp "$WORK/src/SDLPoP.ini" "$D/SDLPoP.ini"
cp "$WORK/src/COPYING" "$D/COPYING"

python3 - "$HERE" "$OUT" "$PIN" "$REPO_REV" "$EPOCH" "$STOVE" "$RELEASE" "$ARCHIVE" <<'EOF'
import hashlib, json, sys
from pathlib import Path
here, out, pin, rev, epoch, stove, release, archive = Path(sys.argv[1]), Path(sys.argv[2]), *sys.argv[3:]
sha = lambda p: hashlib.sha256(Path(p).read_bytes()).hexdigest()
sdk = Path(stove) / "prefix/m68k-amigaos"
inputs = ["lib/libSDL2.a", "lib/libSDL2_image.a", "lib/libGL.a",
          "include/SDL2/SDL.h", "include/SDL2/SDL_image.h"]
drawer = out / "SDLPoP"
receipt = {
    "upstream_commit": pin, "repository_commit": rev, "source_date_epoch": epoch,
    "recipe": {"build-amiga.sh": sha(here / "build-amiga.sh")},
    "patches": {p.name: sha(p) for p in sorted((here / "patches").glob("*.patch"))},
    "compiler_release": release, "compiler_archive_sha256": archive,
    "sdk_inputs": {n: sha(sdk / n) for n in inputs if (sdk / n).is_file()},
    "network": "upstream source fetch only (pinned commit)",
    "executable_sha256": sha(drawer / "prince"),
    "executable_bytes": (drawer / "prince").stat().st_size,
    "settings_sha256": sha(drawer / "SDLPoP.ini"),
    "scope": "Source build only; no game data; no runtime or release pass implied",
}
if archive == "unpublished":
    # no archive to name: the compiler's own programs, by SHA-256
    prefix = Path(stove) / "prefix"
    files = ["bin/m68k-amigaos-gcc", "bin/m68k-amigaos-as", "bin/m68k-amigaos-ld"]
    files += sorted(str(p.relative_to(prefix)) for p in prefix.glob("libexec/gcc/m68k-amigaos/*/cc1"))
    receipt["compiler_files_sha256"] = {f: sha(prefix / f) for f in files if (prefix / f).is_file()}
(out / "receipt.json").write_text(json.dumps(receipt, indent=2) + "\n")
print(json.dumps(receipt, indent=2))
EOF
