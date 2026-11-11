# PiNN

PiNN is an experimental OpenCL development project targeting the
Broadcom VideoCore IV GPU on the Raspberry Pi 3.

## Project goal

Enable OpenCL kernel compilation, memory transfers, and GPU execution
through VC4CL, VC4C, and the Raspberry Pi VideoCore userland interface.

## Components

- `third_party/VC4C` - OpenCL C compiler for VideoCore IV
- `third_party/VC4CL` - OpenCL runtime
- `third_party/VC4CLStdLib` - OpenCL standard library
- `third_party/userland` - Raspberry Pi VideoCore userland interfaces
- `tests` - OpenCL, VCSM, and VCHIQ diagnostic tests
- `icd-test` - Local OpenCL ICD configuration
- `toolchain` - Supporting toolchain files

## Current progress

- OpenCL device discovery works.
- OpenCL kernel compilation works.
- CMA-backed OpenCL buffer allocation and transfers work.
- GPUS VCHIQ service initialization and message submission work.
- QPU kernel completion is not yet working; execution currently times out.

## Platform

Original development environment:
- Raspberry Pi 3 Model B
- Raspberry Pi OS 64-bit
- ARM64 userland
- VideoCore IV GPU

A 32-bit ARM userland compatibility build is the next investigation.

## Source versions

See [UPSTREAM_VERSIONS.md](UPSTREAM_VERSIONS.md) for upstream repositories
and exact starting commits.

Local source modifications are intentionally included in this repository.

## Tests

C source files for diagnostic tests are in `tests/`.
Build instructions will be documented as the project progresses.
