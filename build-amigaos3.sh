#!/bin/sh
# The earlier entry point, kept for its callers: build-amigaos3.sh [SOURCE [OUT]]
# is build-amiga.sh with SDLPOP_SOURCE=SOURCE and OUT=OUT (STOVE as there).
# build-amiga.sh is the build Cradle's stove_build runs.
set -eu
HERE=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
[ $# -ge 1 ] && SDLPOP_SOURCE=$1 && export SDLPOP_SOURCE
[ $# -ge 2 ] && OUT=$2 && export OUT
exec sh "$HERE/build-amiga.sh"
