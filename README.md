# RISC-V Linux Emulator

A Linux-native RISC-V (RV32I + RV32M) emulator with a Qt GUI, inspired by the Windows-only Easy68k debugging experience. Personal project to learn CPU emulation, ISA design, and CMake/C++ project structure.

## Components

- **`core/`** — C++ RV32I/RV32M CPU core: instruction decoding/execution, registers, memory model, exposed to Python via pybind11 (CMake build, with its own test suite)
- **`assembler/`** — example RISC-V `.s` sources; assembling is done via the external `riscv64-unknown-elf-as`/`objcopy` toolchain, invoked by the GUI
- **`gui/`** — PyQt5 GUI: syntax-highlighted code editor, register/memory/stack/disassembly windows, find/replace, assemble-and-run or single-step execution
- **`core/tests/`** — emulator core test cases (one executable per instruction/family, run via `ctest`)
- **`docs/`** — architecture notes (see `docs/ARCHITECTURE.md`)

## Features (current)

- Syntax-highlighted assembly editor; assemble a `.s` file into a binary and run it from the GUI, or step single instructions until halt
- Register window showing live CPU register state
- Memory view in hex format with current program counter highlighted, jump to any address
- Disassembly view of the loaded binary
- Stack view relative to the current stack pointer
- Breakpoints (toggle in the disassembly view), Step Over/Step Out, and a PC history dock
- RV32I base ISA + RV32M (multiply/divide) + FENCE/FENCE.I + CSR instructions

## Planned

See `future_plans.md` for the roadmap: Windows/macOS packaged releases (PyInstaller).

## Status

Active project — core CPU emulation, breakpoints, PC history, settings/layout persistence, and the GUI's register/memory/disassembly/stack views are all functional. First tagged release: [`v0.1.0-beta.1`](https://github.com/imndllnuri/rv32i-emulator/releases), Linux-only (a self-contained AppImage, see below). Windows/macOS packaging is planned for `v0.1.0-beta.2` — see `future_plans.md` and `CHANGELOG.md`.

## Packaged releases (Linux)

Tagged releases (`v*`) automatically build a self-contained AppImage — bundled Python, PyQt5, and RISC-V toolchain, no setup required:

```sh
chmod +x rv32i-emulator-x86_64.AppImage
./rv32i-emulator-x86_64.AppImage
```

To build one locally: `./scripts/build_appimage.sh` (after `./scripts/setup.sh`).

## Building

```sh
python3 -m venv venv && source venv/bin/activate
pip install -r requirements.txt -r requirements-dev.txt
cmake -S core -B core/build -DCMAKE_BUILD_TYPE=Release
cmake --build core/build -j
ctest --test-dir core/build
python gui/main.py
```

`core/CMakeLists.txt` auto-detects `pybind11`'s CMake directory from whichever `python3` is on `PATH`, so no manual `-Dpybind11_DIR` flag is needed as long as the venv above is active. Assembling `.s` files from the GUI requires `riscv64-unknown-elf-as`/`objcopy` on `PATH` (see `CONTRIBUTING.md`).

## Contributing

See `CONTRIBUTING.md`.

## License

MIT — see `LICENSE`.
