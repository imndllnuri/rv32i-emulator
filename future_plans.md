## GUI roadmap

### 1. **Memory Viewer/Editor** — done (`gui/memory_window.py`, `gui/hexdump.py`)
- Show a hex dump of memory contents, with the ability to navigate to specific addresses. ✅
- Highlight memory regions that have been recently accessed or modified.
- Option to edit memory values directly (useful for testing).

### 2. **Disassembly View** — done (`gui/disassembly_window.py`, `gui/disassembler.py`)
- Display the current instruction (at the PC) and nearby instructions in assembly format. ✅
- As you step, highlight the current instruction.
- Show the raw instruction bytes alongside the disassembled mnemonic and operands.

### 3. **Breakpoint Manager** — done (`gui/disassembly_window.py`)
- Double-click a row in the disassembly view to toggle a breakpoint. ✅
- Run pauses when a breakpoint is hit and reports the address in the status bar. ✅
- Still open: a dedicated breakpoints list panel (enable/disable/remove without hunting through disassembly).

### 4. **Stack View** — done (`gui/stack_window.py`)
- Show the contents near the current stack pointer (sp), with annotations for saved registers or return addresses. ✅
- Helpful for understanding function calls and local variables.

### 5. **Program Counter (PC) Tracker** — done (`gui/main_window.py`)
- Permanent PC status-bar label, plus a "PC History" dock (last 50 addresses, double-click to jump). ✅

### 6. **Load/Restart Program** — done (`gui/main_window.py::assemble`, `_reset_program_state`)
- Assemble loads a new binary; New/Open reset CPU state (registers, PC) automatically. ✅

### 7. **Execution Controls** — done (`gui/main_window.py`)
- Run (Continue), Pause, Step, Step Over, Step Out are all implemented. ✅

### 8. **Status Bar / Console** — done (`gui/main_window.py`)
- Permanent PC/instruction-count/state labels, an Output console dock logging run/breakpoint/error messages. ✅
- Still open: no system-call or environment-call logging (ECALL is currently a hard trap, not emulated).

### 9. **Symbol Table / Labels**
- If you parse an ELF file with debug symbols, show addresses as labels (e.g., `main` instead of `0x1000`).
- Helps readability in disassembly and breakpoint lists.

### 10. **Configurable Layout** — done (`gui/main_window.py`, `gui/settings.py`)
- Docking/tabbing of Registers/Memory/Disassembly/Stack/PC History/Output, with a sensible default arrangement. ✅
- Window geometry and dock layout now persist across restarts (QSettings), with a "Reset to Default Layout" escape hatch in Settings if a restored/user-arranged layout gets into a bad state. ✅

### 11. **Instruction Counter / Performance Stats** — partially done
- Live instruction-executed counter in the status bar. ✅
- Still open: instructions-per-second while running.

### 12. **Help / About** — done (`gui/main_window.py::help_contents`, `about`)
- Static keyboard-shortcut reference and About dialog exist. ✅
- In-GUI contextual help: a persistent "?" toolbar button drops into Qt's What's-This mode; every dock and most toolbar actions have explanatory whatsThis/toolTip text. ✅

### 13. **Toolbar/Titlebar polish** — done (`gui/main_window.py`, `gui/main.py`)
- Real app icon (`resources/icons/app.png`), window title reflects the loaded filename and unsaved-changes state. ✅
- High-DPI scaling attributes and a `--file` launch argument. ✅

### 14. **Settings & Recent Files** — done (`gui/settings.py`, `gui/settings_dialog.py`)
- A real Settings dialog (Editor font size, Layout reset) replaces the old stub. ✅
- Recent Files is a real populated menu instead of a stub message. ✅
- Still open: theme configurability (only one hardcoded dark theme exists today).

## Packaging & release roadmap

`v0.1.0-beta.1` shipped Linux-only (see `CHANGELOG.md`/`VERSIONING.md`) — the
original plan gated the first release on Windows packaging too, but Phases
0-4 finished ahead of schedule and there was no reason to hold the release
back for packaging work that hadn't started. Windows/macOS packaging is now
targeted for `v0.1.0-beta.2`.

- **Linux (AppImage)** — done (`scripts/build_appimage.sh`, `scripts/fetch_toolchain.py`, `resources/packaging/appimage/`)
  - Self-contained AppImage bundling a matching CPython, PyQt5, the compiled `rv32i_core` extension, and a vendored RISC-V `as`/`objcopy` (via the xPack toolchain, checksum-verified). ✅
  - Verified to run standalone with no host dependencies (bundled Python/toolchain only) and to actually assemble+run a RISC-V program end to end. ✅
  - Wired into `.github/workflows/release.yml`: tag pushes build and attach the AppImage to a draft GitHub Release. ✅
  - Shipped in `v0.1.0-beta.1`. ✅
- **Linux (Snap)** — deliberately skipped for now. Snapcraft isn't installable/testable in this environment, and strict confinement is known to sometimes block the subprocess calls this app makes to invoke the toolchain; revisit if there's real demand.
- **Windows / macOS (PyInstaller)** — not started, targeted for `v0.1.0-beta.2`. Known risks going in: PyInstaller's static analysis doesn't trace into the compiled `rv32i_core` extension (needs an explicit hidden-import/binary declaration), PyQt5's platform plugin (`platforms/qwindows.dll`, macOS `cocoa`) needs explicit bundling, and macOS ships unsigned in beta-2 (Gatekeeper "right-click → Open" workaround, documented for testers) with paid Apple Developer signing deferred to v1.0.

## ISA roadmap

RV32I base + RV32M (multiply/divide) + FENCE/FENCE.I + CSR instructions (CSRRW/RS/RC/WI/SI/CI) are implemented in `core/`.

Not implemented, future work:
- **A-extension** (atomic memory operations) — not needed until multi-hart or interrupt-driven scenarios matter.
- **F-extension** (floating point) — only worth adding if a test program needs it.

