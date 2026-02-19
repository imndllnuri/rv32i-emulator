#ifndef RISCV_CPU_HPP
#define RISCV_CPU_HPP

#include <array>
#include <cstdint>
#include <stdexcept>
#include <sys/types.h>
#include <vector>

namespace riscv {

class MemoryAccessException : public std::runtime_error {
public:
  explicit MemoryAccessException(const std::string &msg)
      : std::runtime_error(msg) {}
};

// main RV32I CPU class
class CPU {
public:
  // Register indices with ABI names
  enum class Reg : uint32_t {
    zero = 0,
    ra = 1,
    sp = 2,
    gp = 3,
    tp = 4,
    t0 = 5,
    t1 = 6,
    t2 = 7,
    s0 = 8,
    s1 = 9,
    a0 = 10,
    a1 = 11,
    a2 = 12,
    a3 = 13,
    a4 = 14,
    a5 = 15,
    a6 = 16,
    a7 = 17,
    s2 = 18,
    s3 = 19,
    s4 = 20,
    s5 = 21,
    s6 = 22,
    s7 = 23,
    s8 = 24,
    s9 = 25,
    s10 = 26,
    s11 = 27,
    t3 = 28,
    t4 = 29,
    t5 = 30,
    t6 = 31
  };

  constexpr static uint32_t MEMORY_SIZE = 1024 * 1024;
  constexpr static uint32_t TEXT_START = 0x01000;
  constexpr static uint32_t STACK_TOP = 0xEFFFF;

  // Constructor: initializes memory and resets CPU
  CPU();

  // Reset CPU to initial state
  void reset();

  // Load a program (binary data) into memory at a given address
  void load_program(const std::vector<uint8_t> &code,
                    uint32_t address = TEXT_START);

  // Execute one instruction. Returns true if execution should continue,
  // false if the program halted (e.g., ECALL with a0=0 or invalid
  // instruction).
  bool step();

  // run until halt.
  void run();

  // Accessors for register and memory state (for GUI/debugging)
  // register space inspection
  const std::array<uint32_t, 32> registers_state() { return regs; };
  uint32_t get_pc() { return pc; }
  uint8_t read_memory_byte() const;
  uint16_t read_memory_half() const;
  uint32_t read_memory_word() const;
  void write_memory_byte(uint32_t addr, uint8_t value);
  void write_memory_half(uint32_t addr, uint16_t value);
  void write_memory_word(uint32_t addr, uint32_t value);

private:
  // program counter shows the next instruction to be executed.
  uint32_t pc;

  // 32 register
  std::array<uint32_t, 32> regs;

  // memory structure each eight bit MEMORY_SIZE lines.
  std::array<uint8_t, MEMORY_SIZE> memory;

  void validate_address();

  // fetch the current instruction at PC
  uint32_t fetch_instruction();

  // execute a decoded instruction
  void execute_instruction(uint32_t instr);

  // Instruction decoding and execution helpers (by format)
  void execute_r_type(uint32_t instr);
  void execute_i_type(uint32_t instr);
  void execute_s_type(uint32_t instr);
  void execute_b_type(uint32_t instr);
  void execute_u_type(uint32_t instr);
  void execute_j_type(uint32_t instr);

  // Helper to read a signed immediate from an instruction
  int32_t read_imm_i(uint32_t instr) const;
  int32_t read_imm_s(uint32_t instr) const;
  int32_t read_imm_b(uint32_t instr) const;
  int32_t read_imm_u(uint32_t instr) const;
  int32_t read_imm_j(uint32_t instr) const;
};
} // namespace riscv
#endif
