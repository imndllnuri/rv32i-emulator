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

namespace rv32i {
uint32_t make_i_type(uint32_t funct3, uint32_t rs1, uint32_t rd, int32_t imm) {
  uint32_t uimm = static_cast<uint32_t>(imm) & 0xFFF;
  return (uimm << 20) | (rs1 << 15) | (funct3 << 12) | (rd << 7) | 0x13;
}
} // namespace rv32i

int main() {
  using namespace rv32i;

  // XORI x3, x1, 0xFF   (funct3=4)
  uint32_t instr = make_i_type(0x4, 1, 3, 0xFF);

  CPU cpu;
  cpu.reset();

  // Test 1: basic XOR with positive immediate
  cpu.set_register(CPU::Reg::ra, 0x12345678);
  cpu.write_memory_word(cpu.get_pc(), instr);
  cpu.step();
  TEST(cpu.registers_state()[3] == (0x12345678 ^ 0xFF), "0x12345678 ^ 0xFF");

  // Test 2: XOR with zero (unchanged)
  cpu.reset();
  cpu.set_register(CPU::Reg::ra, 0xDEADBEEF);
  cpu.write_memory_word(cpu.get_pc(), make_i_type(0x4, 1, 3, 0));
  cpu.step();
  TEST(cpu.registers_state()[3] == 0xDEADBEEF, "XOR with zero unchanged");

  // Test 3: XOR with all ones (bitwise NOT)
  cpu.reset();
  cpu.set_register(CPU::Reg::ra, 0x12345678);
  cpu.write_memory_word(cpu.get_pc(),
                        make_i_type(0x4, 1, 3, -1)); // imm = -1 (0xFFF)
  cpu.step();
  TEST(cpu.registers_state()[3] == (0x12345678 ^ 0xFFFFFFFF),
       "XOR with -1 flips all bits");

  // Test 4: XOR with itself (should be 0)
  cpu.reset();
  cpu.set_register(CPU::Reg::ra, 0xA5A5A5A5);
  cpu.write_memory_word(
      cpu.get_pc(),
      make_i_type(0x4, 1, 3, 0xA5A5A5A5 & 0xFFF)); // only lower 12 bits matter
  cpu.step();
  TEST(cpu.registers_state()[3] == (0xA5A5A5A5 ^ (0xA5A5A5A5 & 0xFFF)),
       "XOR with lower 12 bits of itself");

  // Test 5: x0 unchanged
  cpu.reset();
  cpu.set_register(CPU::Reg::ra, 1);
  cpu.write_memory_word(cpu.get_pc(), instr);
  cpu.step();
  TEST(cpu.registers_state()[0] == 0, "x0 remains zero");

  std::cout << "All XORI tests PASSED!" << std::endl;
  return 0;
}
