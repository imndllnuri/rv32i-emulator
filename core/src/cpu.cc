#include "../include/cpu.hpp"

namespace riscv {

CPU::CPU() : pc(0), regs{}, memory{} {
  reset(); // if reset does more than just zeroing registers
};

/**
 * Resets the CPU to a known initial state without unloading the program.
 *
 * This function prepares the CPU for (re)starting execution of the currently
 * loaded program. It does NOT clear memory, so the program code and data
 * remain intact – allowing the same program to be run multiple times without
 * reloading.
 *
 * The following steps are performed:
 * 1. ZERO ALL REGISTERS – Every register (x0 through x31) is set to 0.
 *    Even though x0 is hardwired to zero by the ISA, zeroing it explicitly
 *    keeps the state consistent and simplifies debugging.
 * 2. SET THE PROGRAM COUNTER (PC) to `TEXT_START` (0x01000). This is the
 *    conventional entry point where programs are loaded.
 * 3. INITIALIZE THE STACK POINTER (SP, X2) to `STACK_TOP` (0xEFFFF). This
 *    follows the RISC‑V ABI, giving the program a valid stack region. Other
 *    ABI‑specified registers (e.g., global pointer gp) are left zero for now;
 *    they can be added later if required.
 * 4. MEMORY IS UNTOUCHED – The contents of RAM are preserved. This allows
 *    a simple `reset()` + `run()` sequence to restart the program from its
 *    entry point without needing to reload the binary.
 *
 * The function is typically called after loading a program, and can be called
 * again at any time to restart execution.
 */
void CPU::reset() {
  // init regs to 0;
  for (auto &reg : regs) {
    reg = 0;
  }

  // set the pc to program start
  pc = TEXT_START;

  // set the sp which is reg2 to stack top
  regs[static_cast<size_t>(Reg::sp)] = STACK_TOP;

  // we do not clear memory.
}

void CPU::load_program(const std::vector<uint8_t> &code, uint32_t address) {}

bool CPU::step() { return -1; }

void CPU::run() {}

uint8_t read_memory_byte() {
  uint8_t data = 0;
  return data;
}

uint16_t read_memory_half() {
  uint8_t data = 0;
  return data;
}

uint32_t read_memory_word() {
  uint8_t data = 0;
  return data;
}

void write_memory_byte(uint32_t addr, uint8_t value) {}
void write_memory_half(uint32_t addr, uint16_t value) {}
void write_memory_word(uint32_t addr, uint32_t value) {}
} // namespace riscv
