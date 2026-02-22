#include "../include/cpu.hpp"
#include <cstdint>
#include <iostream>

#define TEST(cond, msg)                                                        \
  do {                                                                         \
    if (!(cond)) {                                                             \
      std::cerr << "FAIL: " << msg << " (" << __FILE__ << ":" << __LINE__      \
                << ")" << std::endl;                                           \
      return 1;                                                                \
    }                                                                          \
  } while (0)

namespace riscv {
// Helper to build an I‑type arithmetic immediate instruction
uint32_t make_i_type(uint32_t funct3, uint32_t rs1, uint32_t rd, int32_t imm) {
  uint32_t uimm = static_cast<uint32_t>(imm) & 0xFFF;
  return (uimm << 20) | (rs1 << 15) | (funct3 << 12) | (rd << 7) | 0x13;
}
} // namespace riscv

int main() {
  using namespace riscv;

  // ADDI x3, x1, 5   (funct3=0)
  uint32_t instr = make_i_type(0x0, 1, 3, 5);

  CPU cpu;
  cpu.reset();

  // Test 1: basic addition
  cpu.set_register(CPU::Reg::ra, 10); // x1 = 10
  cpu.write_memory_word(cpu.get_pc(), instr);
  cpu.step();
  TEST(cpu.registers_state()[3] == 15, "10 + 5 = 15");
  TEST(cpu.get_pc() == TEXT_START + 4, "PC advanced by 4");
  TEST(cpu.registers_state()[0] == 0, "x0 remains zero");

  // Test 2: negative immediate (sign‑extended)
  cpu.reset();
  cpu.set_register(CPU::Reg::ra, 10);
  cpu.write_memory_word(cpu.get_pc(),
                        make_i_type(0x0, 1, 3, -5)); // imm = -5 (0xFFB)
  cpu.step();
  TEST(cpu.registers_state()[3] == 5, "10 + (-5) = 5");

  // Test 3: addi with x0 as source
  cpu.reset();
  cpu.write_memory_word(cpu.get_pc(), make_i_type(0x0, 0, 3, 42)); // rs1 = x0
  cpu.step();
  TEST(cpu.registers_state()[3] == 42, "x0 + 42 = 42");

  // Test 4: addi with immediate 0
  cpu.reset();
  cpu.set_register(CPU::Reg::ra, 99);
  cpu.write_memory_word(cpu.get_pc(), make_i_type(0x0, 1, 3, 0));
  cpu.step();
  TEST(cpu.registers_state()[3] == 99, "99 + 0 = 99");

  // Test 5: overflow wraps around
  cpu.reset();
  cpu.set_register(CPU::Reg::ra, 0x7FFFFFFF);
  cpu.write_memory_word(cpu.get_pc(), make_i_type(0x0, 1, 3, 1));
  cpu.step();
  TEST(cpu.registers_state()[3] == 0x80000000,
       "0x7FFFFFFF + 1 wraps to 0x80000000");

  std::cout << "All ADDI tests PASSED!" << std::endl;
  return 0;
}
