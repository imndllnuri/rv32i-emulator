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

  // ORI x3, x1, 0xF0   (funct3=6)
  uint32_t instr = make_i_type(0x6, 1, 3, 0xF0);

  CPU cpu;
  cpu.reset();

  // Test 1: basic OR
  cpu.set_register(CPU::Reg::ra, 0x12345678);
  cpu.write_memory_word(cpu.get_pc(), instr);
  cpu.step();
  TEST(cpu.registers_state()[3] == (0x12345678 | 0xF0), "0x12345678 | 0xF0");

  // Test 2: OR with zero (unchanged)
  cpu.reset();
  cpu.set_register(CPU::Reg::ra, 0xDEADBEEF);
  cpu.write_memory_word(cpu.get_pc(), make_i_type(0x6, 1, 3, 0));
  cpu.step();
  TEST(cpu.registers_state()[3] == 0xDEADBEEF, "OR with zero unchanged");

  // Test 3: OR with all ones (result all ones)
  cpu.reset();
  cpu.set_register(CPU::Reg::ra, 0x12345678);
  cpu.write_memory_word(cpu.get_pc(),
                        make_i_type(0x6, 1, 3, -1)); // imm = -1 (0xFFF)
  cpu.step();
  std::cout << cpu.registers_state()[3] << std::endl;
  TEST(cpu.registers_state()[3] == (0xFFFFFFFF),
       "OR with 0xFFF sets lower 12 bits");

  // Test 4: x0 unchanged
  cpu.reset();
  cpu.set_register(CPU::Reg::ra, 1);
  cpu.write_memory_word(cpu.get_pc(), instr);
  cpu.step();
  TEST(cpu.registers_state()[0] == 0, "x0 remains zero");

  std::cout << "All ORI tests PASSED!" << std::endl;
  return 0;
}
