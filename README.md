
# PiNN

PiNN is an experimental GPU computing project for the Raspberry Pi 3, built around the Broadcom VideoCore IV GPU.

The project integrates [V3DLib](https://github.com/axizo-pi/V3DLib) to develop and execute GPU programs directly on the Raspberry Pi, using its native VideoCore IV hardware.

## Features

- Native VideoCore IV GPU execution
- V3DLib integration
- Automated dependency initialization and configuration
- Reproducible builds using pinned Git submodules
- Automated GPU smoke testing
- Raspberry Pi userland integration

## Hardware and Software

The project has been tested on the following configuration:

| Component | Configuration |
|---|---|
| Board | Raspberry Pi 3 |
| GPU | Broadcom VideoCore IV |
| Architecture | ARMv7 (32-bit) |
| Operating System | Raspberry Pi OS |
| Compiler | GCC |
| Build System | GNU Make and CMake |
| GPU Library | V3DLib |

Other hardware and operating system configurations have not yet been verified.

## Installation

Clone the repository and its submodules:

```bash
git clone --recurse-submodules https://github.com/dkorn3/PiNN.git
cd PiNN
```

Run the installer:

```bash
chmod +x install.sh
./install.sh
```

The installer automatically:

1. Installs required system dependencies.
2. Initializes pinned Git submodules.
3. Applies V3DLib compatibility patches.
4. Builds CmdParameter.
5. Configures and builds Raspberry Pi userland libraries.
6. Generates V3DLib source dependencies.
7. Builds the V3DLib Hello example.
8. Executes the GPU smoke test.

## Manual Build

To build PiNN without running the installer:

```bash
./scripts/build.sh
```

To execute the GPU smoke test separately:

```bash
./scripts/run-smoke-test.sh
```

Alternatively, use the root Makefile:

```bash
make build
make smoke-test
```

## GPU Smoke Test

PiNN includes a GPU smoke test based on the V3DLib Hello example.

The test executes a GPU program on the VideoCore IV and verifies its output.

A successful execution produces:

```text
0: 1
1: 1
2: 1
3: 1
4: 1
5: 1
6: 1
7: 1
8: 1
9: 1
10: 1
11: 1
12: 1
13: 1
14: 1
15: 1
```

Successful execution confirms that the GPU program was compiled, launched, and returned its expected results.

## Project Structure

```text
PiNN/
├── install.sh
├── Makefile
├── scripts/
│   ├── build.sh
│   ├── patch-v3dlib.sh
│   └── run-smoke-test.sh
└── third_party/
    ├── CmdParameter/
    └── V3DLib/
```

Third-party dependencies are maintained as Git submodules and pinned to specific commits to ensure consistent builds.

## Cleaning Build Artifacts

To clean generated build artifacts:

```bash
make clean
```

## License

This project is licensed under the MIT License. See [LICENSE](LICENSE) for details.
