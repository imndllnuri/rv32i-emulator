## GUI roadmap

### 1. **Memory Viewer/Editor** — done (`gui/memory_window.py`, `gui/hexdump.py`)
- Show a hex dump of memory contents, with the ability to navigate to specific addresses. ✅
- Highlight memory regions that have been recently accessed or modified.
- Option to edit memory values directly (useful for testing).

### 2. **Disassembly View** — done (`gui/disassembly_window.py`, `gui/disassembler.py`)
- Display the current instruction (at the PC) and nearby instructions in assembly format. ✅
- As you step, highlight the current instruction.
- Show the raw instruction bytes alongside the disassembled mnemonic and operands.

### 3. **Breakpoint Manager**
- Allow setting/clearing breakpoints on specific addresses (e.g., double-click in the disassembly view).
- A list of active breakpoints that can be enabled/disabled or removed.
- When a breakpoint is hit, pause execution and highlight the line.

### 4. **Stack View** — done (`gui/stack_window.py`)
- Show the contents near the current stack pointer (sp), with annotations for saved registers or return addresses. ✅
- Helpful for understanding function calls and local variables.

### 5. **Program Counter (PC) Tracker**
- A small indicator showing the current PC value, maybe with a button to jump to that address in memory/disassembly.
- Option to show a history of the last few executed PCs (like a call stack).

### 6. **Load/Restart Program**
- Buttons to load a new binary (ELF or raw binary) into memory.
- A **Reset** button to reset the CPU state (registers, PC) and reload the program.

### 7. **Execution Controls**
- **Continue** (already have Run) – runs until breakpoint or halt.
- **Step Over** – step over a function call (if you implement some minimal debug info, or just treat as a single step).
- **Step Out** – run until return from current function.
- **Pause** button to halt a running program.

### 8. **Status Bar / Console**
- Show current simulation state (running, paused, halted) and any error messages.
- Display the last instruction executed or a message like "Hit breakpoint at 0x...".
- Maybe a small log of system calls or console output (if emulating environment calls).

### 9. **Symbol Table / Labels**
- If you parse an ELF file with debug symbols, show addresses as labels (e.g., `main` instead of `0x1000`).
- Helps readability in disassembly and breakpoint lists.

### 10. **Configurable Layout**
- Allow docking or tabbing the various views (registers, memory, disassembly, stack) so the user can arrange them as they like.
- Save/restore window layouts.

### 11. **Instruction Counter / Performance Stats**
- Show how many instructions have been executed so far.
- Optionally, instructions per second if running continuously.

### 12. **Help / About**
- A quick reference for RISC-V instructions or your emulator's shortcuts.

## ISA roadmap

RV32I base + RV32M (multiply/divide) + FENCE/FENCE.I + CSR instructions (CSRRW/RS/RC/WI/SI/CI) are implemented in `core/`.

Not implemented, future work:
- **A-extension** (atomic memory operations) — not needed until multi-hart or interrupt-driven scenarios matter.
- **F-extension** (floating point) — only worth adding if a test program needs it.

