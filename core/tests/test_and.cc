// TEST: Passed all tests.
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

int main() {
  using namespace rv32i;

  // AND x3, x1, x2  (funct7=0, funct3=7, opcode=0x33)
  // Encoding: x0020F1B3
  uint32_t instr = 0x0020F1B3;

  CPU cpu;
  cpu.reset();

  // Test 1: basic AND
  cpu.set_register(CPU::Reg::ra, 0b1010);
  cpu.set_register(CPU::Reg::sp, 0b1100);
  // std::cout << cpu.registers_state()[1] << std::endl;
  cpu.write_memory_word(cpu.get_pc(), instr);
  cpu.step();
  // std::cout << cpu.registers_state()[3] << std::endl;
  TEST(cpu.registers_state()[3] == (0b1010 & 0b1100),
       "0b1010 & 0b1100 = 0b1000");

  // Test 2: AND with zero (zero)
  cpu.reset();
  cpu.set_register(CPU::Reg::ra, 0xDEADBEEF);
  cpu.set_register(CPU::Reg::sp, 0);
  cpu.write_memory_word(cpu.get_pc(), instr);
  cpu.step();
  TEST(cpu.registers_state()[3] == 0, "AND with zero -> zero");

  // Test 3: AND with all ones (unchanged)
  cpu.reset();
  cpu.set_register(CPU::Reg::ra, 0x12345678);
  cpu.set_register(CPU::Reg::sp, 0xFFFFFFFF);
  cpu.write_memory_word(cpu.get_pc(), instr);
  cpu.step();
  TEST(cpu.registers_state()[3] == 0x12345678,
       "AND with all ones -> unchanged");

  // Test 4: AND with itself (unchanged)
  cpu.reset();
  cpu.set_register(CPU::Reg::ra, 0xA5A5A5A5);
  cpu.set_register(CPU::Reg::sp, 0xA5A5A5A5);
  cpu.write_memory_word(cpu.get_pc(), instr);
  cpu.step();
  TEST(cpu.registers_state()[3] == 0xA5A5A5A5, "AND with itself -> unchanged");

  // Test 5: x0 unchanged
  cpu.reset();
  cpu.set_register(CPU::Reg::ra, 1);
  cpu.set_register(CPU::Reg::sp, 2);
  cpu.write_memory_word(cpu.get_pc(), instr);
  cpu.step();
  TEST(cpu.registers_state()[0] == 0, "x0 remains zero");

  std::cout << "All AND tests PASSED!" << std::endl;
  return 0;
}
