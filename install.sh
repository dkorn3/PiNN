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
echo "==> Running VideoCore IV GPU smoke test"
bash "$ROOT/scripts/run-smoke-test.sh"

echo
echo "======================================"
echo "       PiNN installation complete"
echo "======================================"
echo "Build: successful"
echo "GPU smoke test: successful"
