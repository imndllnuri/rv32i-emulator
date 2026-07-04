# Contributing

## Prerequisites

- A C++17 compiler (GCC/Clang on Linux/macOS, MSVC on Windows) and CMake 3.10+.
- Python 3.8+.
- `riscv64-unknown-elf-as` and `riscv64-unknown-elf-objcopy` on `PATH`, only needed to use the GUI's Assemble button (the core/tests don't need it).

## Setup

Always create your own virtual environment locally — never commit or share one, since its
console-script shebangs (`pip`, `pytest`, `cmake`, ...) hardcode an absolute path to wherever
it was created and break the moment the repo is cloned or moved elsewhere.

```sh
git clone <this-repo>
cd rv32i-emulator
python3 -m venv venv
source venv/bin/activate        # .\venv\Scripts\Activate.ps1 on Windows
pip install -r requirements.txt -r requirements-dev.txt
```

## Building and testing

```sh
cmake -S core -B core/build -DCMAKE_BUILD_TYPE=Release
cmake --build core/build -j
ctest --test-dir core/build --output-on-failure
pytest
```

`core/CMakeLists.txt` locates `pybind11`'s CMake config by asking whichever `python3` is first
on `PATH` — make sure your venv is active (or pass `-Dpybind11_DIR=$(python3 -c "import pybind11; print(pybind11.get_cmake_dir())")`
explicitly) before configuring.

Run the GUI directly against a freshly built core with:

```sh
python gui/main.py
```

## Code style / conventions

- C++ core: one file per concern (`cpu.cc`, `decode.cc`, `execute.cpp`, ...), free functions under
  `rv32i::execute::` rather than CPU member functions for the actual per-format instruction logic.
- Every instruction/family gets its own CTest executable under `core/tests/test_*.cc`, using the
  minimal `TEST(cond, msg)` macro already in use — no external test framework. Shared instruction
  encoders live in `core/tests/common/instr_encoders.hpp`; add to that header rather than
  redefining `make_r_type()`-style helpers per test file.
- Python/GUI: PyQt5, one widget per file under `gui/`, `.ui` files edited with Qt Designer live
  under `gui/ui/`.
- Keep `docs/ARCHITECTURE.md`, `README.md`, and `future_plans.md` in sync with reality when you
  land a feature that changes the directory layout or closes a roadmap item.
