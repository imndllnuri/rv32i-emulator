# Architecture

Current directory layout:

```
rv32i-emulator/
├── README.md
├── CONTRIBUTING.md
├── LICENSE
├── future_plans.md
├── docs/
│   └── ARCHITECTURE.md
├── core/                       # C++ RV32I/RV32M CPU core
│   ├── include/
│   │   ├── cpu.hpp
│   │   ├── csr.hpp
│   │   ├── decode.hpp
│   │   ├── exception.hpp
│   │   ├── execute.hpp
│   │   ├── fetch.hpp
│   │   ├── instruction.hpp
│   │   ├── memory.hpp
│   │   └── register.hpp
│   ├── src/
│   │   ├── cpu.cc
│   │   ├── csr.cpp
│   │   ├── decode.cc
│   │   ├── execute.cpp
│   │   ├── fetch.cpp
│   │   └── pycpu.cpp            # pybind11 bindings exposed to the GUI
│   ├── tests/                    # one ctest executable per instruction/family
│   └── main.cc
├── assembler/                    # example RISC-V .s sources (assembled via riscv64-unknown-elf-as)
├── gui/                           # PyQt5 GUI
│   ├── main.py                    # entry point
│   ├── main_window.py             # main window, ties editor/registers/memory/run controls together
│   ├── code_editor.py             # source editor widget
│   ├── syntax_highlighter.py      # RISC-V assembly syntax highlighting
│   ├── disassembler.py            # binary -> mnemonic disassembly
│   ├── disassembly_window.py      # disassembly view
│   ├── register_window.py         # live register state
│   ├── memory_window.py           # hex dump + jump-to-address
│   ├── stack_window.py            # stack pointer-relative view
│   ├── hexdump.py                 # hex dump formatting helper
│   ├── find_dialog.py / replace_dialog.py
│   └── ui/                        # Qt Designer .ui files for each window
├── resources/                     # static assets (icons, etc.)
└── tests/                         # placeholder for Python-level integration tests
```

## Notes

- The CPU core is C++ (CMake build), exposed to Python through pybind11 (`core/src/pycpu.cpp`).
- The GUI assembles `.s` files using the external `riscv64-unknown-elf-as`/`objcopy` toolchain rather than a custom assembler.
- `core/build/` is a local CMake build directory and is gitignored.
- See `future_plans.md` for the roadmap.
