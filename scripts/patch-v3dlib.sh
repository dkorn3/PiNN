#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
V3DLIB="$ROOT/third_party/V3DLib"
CONFIG="$V3DLIB/script/mak/config.mak"
MAKEFILE="$V3DLIB/Makefile"

python3 - "$CONFIG" "$MAKEFILE" <<'PY'
from pathlib import Path
import sys

config = Path(sys.argv[1])
makefile = Path(sys.argv[2])

c = config.read_text()
m = makefile.read_text()

# Make the V3DLib root independent of the machine-specific checkout path.
base_lines = (
    "BASE=~/projects/V3DLib",
    "BASE=$(HOME)/V3DLib",
    "BASE := $(abspath $(dir $(lastword $(MAKEFILE_LIST)))/../..)",
)

found_base = [line for line in base_lines if line in c]
if len(found_base) > 1:
    raise SystemExit("Unexpected multiple BASE definitions; refusing to edit.")

if found_base and found_base[0] != base_lines[2]:
    c = c.replace(found_base[0], base_lines[2], 1)
elif not found_base:
    raise SystemExit("Could not find a recognized BASE definition in config.mak.")

# Remove obsolete /opt/vc paths only when they occur as exact lines.
for line in (
    "CXX_FLAGS += -I /opt/vc/include",
    "LIBS += -L /opt/vc/lib -l bcm_host",
):
    c = c.replace(line + "\n", "")

# Disable the upstream installer, which clones and builds an unpinned
# CmdParameter checkout. PiNN manages the pinned submodule itself.
installer = "\t@./script/install.sh\n"
if installer in m:
    m = m.replace(installer, "", 1)

config.write_text(c)
makefile.write_text(m)

print("V3DLib compatibility settings applied.")
PY
