# Changelog

All notable changes to this project are documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/),
and this project adheres to [Semantic Versioning](https://semver.org/) (see
[VERSIONING.md](VERSIONING.md)).

## [Unreleased]

### Added
- 13 example RISC-V programs under `assembler/` (fibonacci, factorial
  iterative/recursive, gcd, array sum/max/reverse, bubble sort, string
  length, multiplication table, bitwise ops, popcount, CSR scratch demo),
  plus `assembler/README.md` indexing all of them with expected results.
- An **Examples** menu in the GUI listing every bundled `.s` file — click
  one to load it straight into the editor (no more manually browsing to
  `assembler/` via File > Open). Hovering an entry shows a one-line
  description in the status bar. The AppImage now bundles `assembler/`'s
  `.s` sources so this menu isn't empty in packaged builds.

### Fixed
- GUI's Assemble step used `-march=rv32im`, which modern binutils rejects
  for any CSR instruction (`extension 'zicsr' required`) — CSR support was
  fully implemented and unit-tested in the core (`core/tests/test_csrr*.cc`)
  but unreachable from the GUI. Now assembles with `-march=rv32im_zicsr`.

## [0.1.0-beta.1] - 2026-07-11

First public release. Everything below was already implemented as a personal
project before this release pass; this version is the "made presentable and
installable" milestone, not a from-scratch rewrite.

### Added

- Self-contained Linux AppImage, attached automatically to tagged GitHub
  Releases — bundled CPython, PyQt5, the compiled `rv32i_core` extension, and
  a vendored RISC-V `as`/`objcopy` toolchain, so nothing needs to be
  preinstalled. Windows/macOS packages are not part of this release; see
  `future_plans.md`.
- `LICENSE` (MIT) and `CONTRIBUTING.md`.
- One-command bootstrap (`scripts/setup.sh` / `scripts/setup.ps1`): creates
  the venv, installs dependencies, builds the C++ core, and runs both test
  suites.
- GitHub Actions CI (`.github/workflows/ci.yml`): every push/PR to `main` is
  built and tested on Linux, Windows, and macOS.
- A Python test layer (`tests/`) on top of the existing C++ core tests —
  `tests/test_pycpu.py` against the `rv32i_core` pybind11 module directly,
  plus `pytest-qt` GUI smoke tests. Also closed a real coverage gap: the
  store instructions (`sb`/`sh`/`sw`) had zero test coverage before this.
- Settings persistence (`gui/settings.py`, QSettings-backed): window
  geometry, dock layout, editor font size, and a recent-files list all
  survive restarts, with a "Reset to Default Layout" escape hatch.
- A real Settings dialog, a populated Recent Files menu, tooltips and
  What's-This contextual help text on every dock and toolbar action, a real
  app icon, a dynamic window title (filename + unsaved-changes marker),
  High-DPI scaling, and a `--file` CLI argument.
- Resizable Find/Replace dialogs and a working `replace_current()`/
  `replace_all()` implementation (previously stubs that only touched a
  status label).
- Alternating row colors on the Register/Disassembly views; the Stack view
  now highlights the line containing the current stack pointer.
- `VERSION` file as the single source of truth for the app version, shown in
  the About dialog.

### Fixed

- **Integer-overflow bounds check causing a segfault in `Memory`**
  (`core/include/memory.hpp`): `read_half`/`read_word`/`write_half`/
  `write_word` checked bounds as `addr + N >= MEMORY_SIZE`, which overflows
  and wraps past 0 for `addr` near `UINT32_MAX`, silently passing the check
  and reading/writing out of bounds. Added a permanent regression test
  (`core/tests/test_memory_bounds.cc`).
- **Every test segfaulted on Windows**: `Memory` held its 1 MiB buffer as a
  stack-allocated `std::array`; MSVC's default thread stack is 1 MiB
  (Linux/macOS default to 8 MiB, which is why this never showed up there).
  Switched to a heap-allocated `std::vector`.
- **`rv32i_core` failed to import on Windows**: CMake's default Windows
  generator (Visual Studio, multi-config) places build output in
  `core/build/Release/` instead of directly in `core/build/`. Fixed with a
  generator-agnostic build-directory search used by both `gui/main_window.py`
  and `tests/conftest.py`.
- **Windows CI install failure**: `requirements.txt` pinned
  `PyQt5-Qt5==5.15.18`, which has no Windows wheel (Windows tops out at
  5.15.2). Only the top-level `PyQt5` package is pinned now; pip resolves the
  platform-appropriate sub-package versions.
- **AppImage packaging: bundled Python version must match whatever compiled
  `rv32i_core`.** pybind11 extensions are ABI-tied to an exact CPython minor
  version; the build now derives the bundled interpreter version dynamically
  from whatever `python3` compiled `core/`, instead of hardcoding it.
- A race where the Memory/Stack dock "Go"/"Refresh" buttons stayed clickable
  while a background thread was stepping the CPU during Run.

### Changed

- Deleted 12 dead 0-byte scaffold files (`core/include/device.hpp`,
  `core/include/devices/*`, `core/src/device/*`, `core/instructions/*`) that
  were silently globbed into the build as no-ops, hiding "did I forget to
  implement this" bugs.
- Deduplicated the ~10 instruction-encoder helpers repeated across every
  `core/tests/test_*.cc` file into `core/tests/common/instr_encoders.hpp`.
- `core/CMakeLists.txt` now auto-detects `pybind11`'s CMake dir from the
  active `python3` — a fresh clone previously failed to configure without a
  manual `-Dpybind11_DIR` flag.
