#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
V3DLIB="$ROOT/third_party/V3DLib"
CMDPARAMETER="$ROOT/third_party/CmdParameter"

echo "==> Initializing pinned submodules"
git -C "$ROOT" submodule update --init --recursive

echo "==> Applying V3DLib compatibility settings"
"$ROOT/scripts/patch-v3dlib.sh"

echo "==> Building CmdParameter debug library"
make -C "$CMDPARAMETER" clean
make -C "$CMDPARAMETER" DEBUG=1 all

echo "==> Building CmdParameter release library"
make -C "$CMDPARAMETER" DEBUG=0 all

echo "==> Configuring Raspberry Pi userland with CMake"
USERLAND="$V3DLIB/extern/userland"

cmake -S "$USERLAND" -B "$USERLAND/build" \
    -DARM64=OFF \
    -DVIDEOCORE_BUILD_DIR="$USERLAND/build"

echo "==> Building VCSM and BCM host libraries"
cmake --build "$USERLAND/build" \
    --target vcsm bcm_host \
    --parallel "$(nproc)"

test -f "$USERLAND/build/lib/libvcsm.a"
test -f "$USERLAND/build/lib/libbcm_host.so"

echo "==> Userland libraries verified"

echo "==> Generating V3DLib source dependencies"
(
    cd "$V3DLIB"
    bash script/gen.sh
)

echo "==> Building V3DLib Hello"
make -C "$V3DLIB" DEBUG=1 Hello

echo
echo "Build completed."
echo "Executable: $V3DLIB/obj/debug/bin/Hello"
