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
uint32_t make_shift_i_type(uint32_t funct3, uint32_t funct7, uint32_t rs1,
                           uint32_t rd, uint32_t shamt) {
  uint32_t imm = (funct7 << 5) | (shamt & 0x1F);
  return (imm << 20) | (rs1 << 15) | (funct3 << 12) | (rd << 7) | 0x13;
}
} // namespace rv32i

int main() {
  using namespace rv32i;

  // SRLI x3, x1, 1   (funct3=5, funct7=0)
  uint32_t instr = make_shift_i_type(0x5, 0x00, 1, 3, 1);

  CPU cpu;
  cpu.reset();

  // Test 1: logical shift right by 1 (positive number)
  cpu.set_register(CPU::Reg::ra, 0x12345678);
  cpu.write_memory_word(cpu.get_pc(), instr);
  cpu.step();
  TEST(cpu.registers_state()[3] == (0x12345678 >> 1),
       "0x12345678 >> 1 (logical)");

  // Test 2: shift by 0 unchanged
  cpu.reset();
  cpu.set_register(CPU::Reg::ra, 0xDEADBEEF);
  cpu.write_memory_word(cpu.get_pc(), make_shift_i_type(0x5, 0x00, 1, 3, 0));
  cpu.step();
  TEST(cpu.registers_state()[3] == 0xDEADBEEF, "shift by 0 unchanged");

  // Test 3: shift by 31 of 0x80000000 -> 1
  cpu.reset();
  cpu.set_register(CPU::Reg::ra, 0x80000000);
  cpu.write_memory_word(cpu.get_pc(), make_shift_i_type(0x5, 0x00, 1, 3, 31));
  cpu.step();
  TEST(cpu.registers_state()[3] == 1, "0x80000000 >> 31 = 1");

  // Test 4: shift amount modulo 32 (33 -> 1)
  cpu.reset();
  cpu.set_register(CPU::Reg::ra, 0x80000000);
  cpu.write_memory_word(cpu.get_pc(), make_shift_i_type(0x5, 0x00, 1, 3, 33));
  cpu.step();
  TEST(cpu.registers_state()[3] == 0x40000000, "shift by 33 = shift by 1");

  // Test 5: x0 unchanged
  cpu.reset();
  cpu.set_register(CPU::Reg::ra, 1);
  cpu.write_memory_word(cpu.get_pc(), instr);
  cpu.step();
  TEST(cpu.registers_state()[0] == 0, "x0 remains zero");

  std::cout << "All SRLI tests PASSED!" << std::endl;
  return 0;
}
