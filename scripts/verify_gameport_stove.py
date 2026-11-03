#!/usr/bin/env python3
"""Fail closed unless a game-port build uses an approved compiler stove.

    python3 scripts/verify_gameport_stove.py --stove STOVE

Prints "RELEASE ARCHIVE-SHA256" and exits 0 for:

  16.2.0-open3  OpenAmigaGCC, its archive's SHA-256 in the stove's
                STOVE.json (Cradle's stoves) or provenance.json (a staged
                stove)
  16.2.0-open4  the same, its own archive
  16.2.0-open5  GFOpenAmigaGCC (Cradle's os32-gcc16 stove since the evening
                of 2 November 2026): no archive is published, so it is taken
                by STOVE.json's release and the compiler's own --version, and
                ARCHIVE-SHA256 is "unpublished" (build-amiga.sh's receipt then
                records the SHA-256 of the compiler's own programs)

Anything else stops the build. --release limits it to one of them.
MIT, Copyright (c) 2026 AmiCradle Team.
"""
from __future__ import annotations

import argparse
import json
import subprocess
import sys
from pathlib import Path

KNOWN = {
    "9fcc136ab59e3bb5b91d2764d0bf290b709d72d7380be4ce0cdf005edf8eff21": "16.2.0-open3",
    "495319a2ffd803278f8827cad70074029c7aa9ecbe05d037e8646ba43ab94ab2": "16.2.0-open4",
}
OPEN5 = "GFOpenAmigaGCC 16.2.0-open5"


def verify(stove: Path) -> tuple[str, str]:
    got = release = None
    if (stove / "STOVE.json").is_file():
        info = json.loads((stove / "STOVE.json").read_text())
        got = (info.get("compilerArchive") or {}).get("sha256")
        release = info.get("release")
    if got is None and (stove / "provenance.json").is_file():
        got = json.loads((stove / "provenance.json").read_text()).get("archive_sha256")
    if got in KNOWN:
        return KNOWN[got], got
    if got is None and release == OPEN5:
        cc = stove / "prefix/bin/m68k-amigaos-gcc"
        version = subprocess.run([str(cc), "--version"], capture_output=True, text=True).stdout
        if f"({OPEN5})" not in version:
            raise ValueError(f"STOVE.json says {release}, the compiler says {version.splitlines()[:1]}")
        return "16.2.0-open5", "unpublished"
    raise ValueError(f"{stove} is not an OpenAmigaGCC 16.2.0-open3 or open4 or a "
                     f"GFOpenAmigaGCC 16.2.0-open5 stove (compiler archive {got}, release {release})")


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("--stove", required=True, type=Path)
    parser.add_argument("--release", choices=("16.2.0-open3", "16.2.0-open4", "16.2.0-open5"))
    args = parser.parse_args()
    try:
        release, archive = verify(args.stove.resolve())
    except (OSError, ValueError, KeyError, TypeError) as exc:
        parser.exit(2, f"STOVE_GATE_FAILED: {exc}\n")
    if args.release and release != args.release:
        parser.exit(2, f"STOVE_GATE_FAILED: the stove is {release}, not {args.release}\n")
    print(release, archive)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
