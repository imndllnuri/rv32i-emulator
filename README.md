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
- RV32I base ISA + RV32M (multiply/divide) + FENCE/FENCE.I + CSR instructions

## Planned

See `future_plans.md` for the roadmap: breakpoint manager, PC history, configurable layout.

## Status

Active personal project — core CPU emulation and the GUI's register/memory/disassembly/stack views are functional; breakpoints and layout persistence are still in progress.
