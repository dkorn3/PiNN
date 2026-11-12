#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
BIN="$ROOT/third_party/V3DLib/obj/debug/bin/Hello"

if [[ ! -x "$BIN" ]]; then
    echo "Hello executable not found. Run 'make build' first." >&2
    exit 1
fi

echo "Running V3DLib VideoCore IV Hello example..."

if [[ "$EUID" -eq 0 ]]; then
    exec "$BIN"
else
    exec sudo "$BIN"
fi
