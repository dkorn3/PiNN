# PiNN

PiNN is an experimental C++ computing library for the Raspberry Pi, providing matrix multiplication through CPU and GPU backends.

A C++ matrix multiplication library supporting ARM CPU execution and VideoCore IV QPU acceleration via [V3DLib](https://github.com/axizo-pi/V3DLib).

PiNN provides a unified C++ interface for both backends and can be installed system-wide for use by external applications.

## Features

- CPU matrix multiplication using standard C++.
- GPU matrix multiplication using the VideoCore IV QPU.
- Unified C++ API for CPU and GPU operations.
- Static library (`libpinn.a`).
- System-wide header and library installation.
- `pkg-config` integration for external projects.
- Automated dependency initialization and configuration.
- Reproducible builds using pinned Git submodules.
- Automated GPU smoke testing.
- CPU and GPU examples.

## Hardware and Software

The project has been tested on the following configuration:

| Component | Configuration |
|---|---|
| Board | Raspberry Pi 3 Model B |
| GPU | Broadcom VideoCore IV |
| Architecture | ARMv7 (32-bit) |
| Operating System | Raspberry Pi OS |
| Compiler | GCC |
| Language Standard | C++17 |
| Build System | GNU Make and CMake |
| GPU Library | V3DLib |

The CPU implementation uses standard C++ and is designed to be portable. The GPU implementation currently targets compatible VideoCore IV hardware and its supporting software stack.

Other hardware and operating system configurations have not yet been verified.

## Installation

Clone the repository and its submodules:

```bash
git clone --recurse-submodules https://github.com/dkorn3/PiNN.git
cd PiNN
