
#!/usr/bin/env bash
set -Eeuo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

echo "======================================"
echo "          PiNN Installer"
echo "======================================"

# Make sure this is the PiNN repository.
if [[ ! -f "$ROOT/.gitmodules" || ! -f "$ROOT/scripts/build.sh" ]]; then
    echo "Error: install.sh must be inside the PiNN repository."
    exit 1
fi

# This installer targets Debian-based Raspberry Pi OS.
if ! command -v apt-get >/dev/null 2>&1 || [[ ! -f /etc/debian_version ]]; then
    echo "Error: This installer requires Raspberry Pi OS or Debian."
    exit 1
fi

if [[ -r /proc/device-tree/model ]]; then
    MODEL="$(tr -d '\0' < /proc/device-tree/model)"
    echo "Detected device: $MODEL"

    if [[ "$MODEL" != *"Raspberry Pi"* ]]; then
        echo "Warning: This device does not appear to be a Raspberry Pi."
    fi
fi

# Use sudo when not already running as root.
if [[ "$EUID" -eq 0 ]]; then
    SUDO=""
else
    if ! command -v sudo >/dev/null 2>&1; then
        echo "Error: sudo is required to install system dependencies."
        exit 1
    fi
    SUDO="sudo"
fi

echo
echo "==> Installing system dependencies"

$SUDO apt-get update
$SUDO apt-get install -y \
    build-essential \
    cmake \
    git \
    python3 \
    pkg-config \
    libdrm-dev \
    libgbm-dev \
    libudev-dev \
    libwayland-dev \
    libx11-dev \
    libxext-dev \
    libxfixes-dev \
    libxdamage-dev \
    libxrandr-dev \
    libxcb1-dev \
    libxcb-dri3-dev \
    libxcb-present-dev \
    libxcb-sync-dev \
    libxshmfence-dev \
    libxxf86vm-dev

echo
echo "==> Building PiNN and its pinned dependencies"
bash "$ROOT/scripts/build.sh"

echo
echo "==> Building PiNN library and examples"
make -C "$ROOT" all

echo
echo "==> Running VideoCore IV GPU smoke test"
bash "$ROOT/scripts/run-smoke-test.sh"

echo
echo "==> Installing PiNN headers, libraries, and pkg-config metadata"

# Public headers
$SUDO install -D -m 644 \
    "$ROOT/include/pinn/cpu/matmul.h" \
    /usr/local/include/pinn/cpu/matmul.h

$SUDO install -D -m 644 \
    "$ROOT/include/pinn/gpu/matmul.h" \
    /usr/local/include/pinn/gpu/matmul.h

# PiNN library
$SUDO install -D -m 644 \
    "$ROOT/build/lib/libpinn.a" \
    /usr/local/lib/libpinn.a

# GPU dependency libraries
$SUDO install -D -m 644 \
    "$ROOT/third_party/V3DLib/obj/debug/libv3dlib.a" \
    /usr/local/lib/pinn/libv3dlib.a

$SUDO install -D -m 644 \
    "$ROOT/third_party/CmdParameter/obj-debug/libCmdParameter.a" \
    /usr/local/lib/pinn/libCmdParameter.a

$SUDO install -D -m 644 \
    "$ROOT/third_party/V3DLib/extern/userland/build/lib/libvcsm.a" \
    /usr/local/lib/pinn/libvcsm.a

$SUDO install -D -m 644 \
    "$ROOT/third_party/V3DLib/obj/mesa/bin/libmesa.a" \
    /usr/local/lib/pinn/libmesa.a

# pkg-config metadata
$SUDO install -D -m 644 \
    "$ROOT/pkgconfig/pinn.pc" \
    /usr/local/lib/pkgconfig/pinn.pc

echo
echo "======================================"
echo "       PiNN installation complete"
echo "======================================"
echo "Build: successful"
echo "GPU smoke test: successful"
echo "Library: /usr/local/lib/libpinn.a"
echo "GPU dependencies: /usr/local/lib/pinn/"
echo "Headers: /usr/local/include/pinn/"
echo "pkg-config: /usr/local/lib/pkgconfig/pinn.pc"

