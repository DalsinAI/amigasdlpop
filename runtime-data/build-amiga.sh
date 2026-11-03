#!/bin/sh
# Stages the Prince of Persia game data for a lab test, for Cradle's
# stove_build (target runtime-data, STOVE= and OUT= set): upstream's data/
# drawer at the pinned commit (gameports/catalog.json) as one plain ustar
# archive, and untar, the small Amiga unpacker that puts it beside the program
# (from amigapowermanga; OS 3.2 has no archiver).
# The data is the original 1989 game's (Jordan Mechner / Broderbund), not
# SDLPoP's: it is used in labs only. It is not committed, and the package
# (package/build-amiga.sh) leaves it out, as gameports/catalog.json's
# contentPolicy and STANDALONE.md say: the user supplies it.
#   STOVE          the stove (default ~/AmiCradle/stoves/os32-gcc16, then
#                  ~/AmigaChrome/stoves/os32-gcc16)
#   OUT            the output directory (default build-data)
#   SDLPOP_SOURCE  a local upstream checkout holding the pinned commit;
#                  without it the pinned commit is fetched from upstream
# Outputs: sdlpop-data.tar (data/... as upstream has it), untar (Amiga
# program: "untar sdlpop-data.tar <drawer>"), SHA256SUMS (one line a data
# file), data-receipt.json.
# MIT, Copyright (c) 2026 AmiCradle Team.
set -eu

HERE=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
ROOT=$(dirname "$HERE")
if [ -z "${STOVE:-}" ]; then
    STOVE="$HOME/AmiCradle/stoves/os32-gcc16"
    [ -d "$STOVE" ] || STOVE="$HOME/AmigaChrome/stoves/os32-gcc16"
fi
OUT=${OUT:-"$ROOT/build-data"}
CC="$STOVE/prefix/bin/m68k-amigaos-gcc"
mkdir -p "$OUT"

PIN=$(python3 -c 'import json,sys; print(json.load(open(sys.argv[1]))["ports"]["sdlpop"]["source"]["commit"])' "$ROOT/gameports/catalog.json")
URL=$(python3 -c 'import json,sys; print(json.load(open(sys.argv[1]))["ports"]["sdlpop"]["source"]["url"])' "$ROOT/gameports/catalog.json")
WORK=$(mktemp -d "${TMPDIR:-/tmp}/sdlpop-data.XXXXXX")
trap 'rm -rf "$WORK"' EXIT INT TERM
if [ -n "${SDLPOP_SOURCE:-}" ]; then
    SRC=$SDLPOP_SOURCE
else
    SRC="$WORK/upstream"
    git init -q "$SRC"
    git -C "$SRC" fetch -q --depth 1 "$URL" "$PIN"
fi
EPOCH=$(git -C "$SRC" show -s --format=%ct "$PIN")
mkdir -p "$WORK/tree"
git -C "$SRC" archive "$PIN" data | tar -xf - -C "$WORK/tree"

LIST="$WORK/list"
(cd "$WORK/tree" && find data -type f -print | LC_ALL=C sort) > "$LIST"
(cd "$WORK/tree" && while read -r f; do sha256sum "$f"; done < "$LIST") > "$OUT/SHA256SUMS"
# the same bytes from the same files: names sorted, owners and times fixed
(cd "$WORK/tree" && tar --format=ustar --no-recursion --owner=0 --group=0 --numeric-owner \
    --mode='u=rw,go=r' --mtime="@$EPOCH" -cf "$OUT/sdlpop-data.tar" -T "$LIST")

"$CC" --version | head -n 1
"$CC" -noixemul -m68040 -O2 -Wall -Wno-stringop-truncation \
    -ffile-prefix-map="$HERE"=. -o "$OUT/untar" "$HERE/untar.c"
[ "$(od -An -tx1 -N4 "$OUT/untar" | tr -d ' \n')" = "000003f3" ] || { echo "untar is not an Amiga HUNK executable"; exit 2; }

python3 - "$OUT" "$PIN" <<'PY'
import hashlib, json, sys
from pathlib import Path
out, pin = Path(sys.argv[1]), sys.argv[2]
sha = lambda p: hashlib.sha256(Path(p).read_bytes()).hexdigest()
sums = (out / "SHA256SUMS").read_bytes()
receipt = {
    "upstream_commit": pin,
    "files": len(sums.splitlines()),
    "sha256sums_sha256": hashlib.sha256(sums).hexdigest(),
    "archive_sha256": sha(out / "sdlpop-data.tar"),
    "archive_bytes": (out / "sdlpop-data.tar").stat().st_size,
    "untar_sha256": sha(out / "untar"),
    "use": "lab tests only; the original game's data, not shipped in the package",
}
(out / "data-receipt.json").write_text(json.dumps(receipt, indent=2) + "\n")
print(json.dumps(receipt, indent=2))
PY
