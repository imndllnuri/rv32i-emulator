#include "../include/cpu.hpp"
#include "common/instr_encoders.hpp"
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

  CPU cpu;
  cpu.reset();

  // BGEU x1, x2, +8   (funct3=7)
  uint32_t instr = make_b_type(0x7, 1, 2, 8);

  // Test 1: 5 >= 3 -> taken
  cpu.set_register(CPU::Reg::ra, 5);
  cpu.set_register(CPU::Reg::sp, 3);
  cpu.write_memory_word(cpu.get_pc(), instr);
  cpu.step();
  TEST(cpu.get_pc() == TEXT_START + 8, "BGEU taken (5 >= 3)");

  // Test 2: 3 >= 5 false -> not taken
  cpu.reset();
  cpu.set_register(CPU::Reg::ra, 3);
  cpu.set_register(CPU::Reg::sp, 5);
  cpu.write_memory_word(cpu.get_pc(), instr);
  cpu.step();
  TEST(cpu.get_pc() == TEXT_START + 4, "BGEU not taken (3 < 5)");

  // Test 3: unsigned: -1 (0xFFFFFFFF) >= 1 -> true
  cpu.reset();
  cpu.set_register(CPU::Reg::ra, (uint32_t)-1);
  cpu.set_register(CPU::Reg::sp, 1);
  cpu.write_memory_word(cpu.get_pc(), instr);
  cpu.step();
  TEST(cpu.get_pc() == TEXT_START + 8, "BGEU taken (0xFFFFFFFF >= 1)");

  // Test 4: 1 >= -1 false (1 < 0xFFFFFFFF)
  cpu.reset();
  cpu.set_register(CPU::Reg::ra, 1);
  cpu.set_register(CPU::Reg::sp, (uint32_t)-1);
  cpu.write_memory_word(cpu.get_pc(), instr);
  cpu.step();
  TEST(cpu.get_pc() == TEXT_START + 4, "BGEU not taken (1 < 0xFFFFFFFF)");

  // Test 5: equal -> taken
  cpu.reset();
  cpu.set_register(CPU::Reg::ra, 0x80000000);
  cpu.set_register(CPU::Reg::sp, 0x80000000);
  cpu.write_memory_word(cpu.get_pc(), instr);
  cpu.step();
  TEST(cpu.get_pc() == TEXT_START + 8, "BGEU taken (equal)");

  std::cout << "All BGEU tests PASSED!" << std::endl;
  return 0;
}
