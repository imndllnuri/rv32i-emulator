#ifndef RISCV_CPU_HPP
#define RISCV_CPU_HPP

#include "constants.hpp"
#include "csr.hpp"
#include "decode.hpp"
#include "memory.hpp"
#include "register.hpp"
#include <array>
#include <cstdint>
#include <vector>

namespace rv32i {

class CPU {
public:
  constexpr static uint32_t MEMORY_SIZE = rv32i::MEMORY_SIZE;
  constexpr static uint32_t TEXT_START = rv32i::TEXT_START;
  constexpr static uint32_t STACK_TOP = rv32i::STACK_TOP;

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

  CPU();

  // we reset to cpu to initial state
  void reset();

  // we load program into address = text start, with the code.
  void load_program(const std::vector<uint8_t> &code,
                    uint32_t address = TEXT_START);

  // step each instruction while it does not halt
  bool step();

  // run until end stop if halt.
  void run();

  // Debug accessors, print register state.
  const std::array<uint32_t, 32> registers_state() const {
    return regs.get_array();
  }

  uint32_t get_pc() const { return pc; }
  uint8_t read_memory_byte(uint32_t addr) const { return mem.read_byte(addr); }
  uint16_t read_memory_half(uint32_t addr) const { return mem.read_half(addr); }
  uint32_t read_memory_word(uint32_t addr) const { return mem.read_word(addr); }

  void write_memory_byte(uint32_t addr, uint8_t value) {
    mem.write_byte(addr, value);
  }
  void write_memory_half(uint32_t addr, uint16_t value) {
    mem.write_half(addr, value);
  }
  void write_memory_word(uint32_t addr, uint32_t value) {
    mem.write_word(addr, value);
  }

  // testing purposes
  void set_register(Reg r, uint32_t value) {
    regs.write(static_cast<uint32_t>(r), value);
  }

  uint32_t read_csr(uint16_t addr) const { return csrs.read(addr); }
  void write_csr(uint16_t addr, uint32_t value) { csrs.write(addr, value); }

private:
  RegisterFile regs;
  Memory mem;
  CsrFile csrs;
  uint32_t pc;

  // Validation stub (not implemented in original)
  void validate_address() {}

  // Internal fetch/execute
  uint32_t fetch_instruction();
  uint32_t execute_instruction(uint32_t instr, uint32_t current_pc);

  // Per‑format execution stubs (preserved)
  uint32_t execute_r_type(const DecodedInstruction &d_instr,
                          uint32_t current_pc);
  uint32_t execute_i_type(const DecodedInstruction &d_instr,
                          uint32_t current_pc);
  uint32_t execute_s_type(const DecodedInstruction &d_instr,
                          uint32_t current_pc);
  uint32_t execute_b_type(const DecodedInstruction &d_instr,
                          uint32_t current_pc);
  uint32_t execute_u_type(const DecodedInstruction &d_instr,
                          uint32_t current_pc);
  uint32_t execute_j_type(const DecodedInstruction &d_instr,
                          uint32_t current_pc);
};

} // namespace rv32i

#endif
