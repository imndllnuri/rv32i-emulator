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

  // BLT x1, x2, +8   (funct3=4)
  uint32_t instr = make_b_type(0x4, 1, 2, 8);

  // Test 1: rs1 < rs2 (3 < 5) -> taken
  cpu.set_register(CPU::Reg::ra, 3);
  cpu.set_register(CPU::Reg::sp, 5);
  cpu.write_memory_word(cpu.get_pc(), instr);
  cpu.step();
  TEST(cpu.get_pc() == TEXT_START + 8, "BLT taken (3 < 5)");

  // Test 2: rs1 >= rs2 (5 < 3 false) -> not taken
  cpu.reset();
  cpu.set_register(CPU::Reg::ra, 5);
  cpu.set_register(CPU::Reg::sp, 3);
  cpu.write_memory_word(cpu.get_pc(), instr);
  cpu.step();
  TEST(cpu.get_pc() == TEXT_START + 4, "BLT not taken (5 >= 3)");

  // Test 3: signed comparison: -5 < 3 -> taken
  cpu.reset();
  cpu.set_register(CPU::Reg::ra, (uint32_t)-5);
  cpu.set_register(CPU::Reg::sp, 3);
  cpu.write_memory_word(cpu.get_pc(), instr);
  cpu.step();
  TEST(cpu.get_pc() == TEXT_START + 8, "BLT taken (-5 < 3)");

  // Test 4: 3 < -5 false -> not taken
  cpu.reset();
  cpu.set_register(CPU::Reg::ra, 3);
  cpu.set_register(CPU::Reg::sp, (uint32_t)-5);
  cpu.write_memory_word(cpu.get_pc(), instr);
  cpu.step();
  TEST(cpu.get_pc() == TEXT_START + 4, "BLT not taken (3 >= -5)");

  // Test 5: eşit (3 < 3 false) -> not taken
  cpu.reset();
  cpu.set_register(CPU::Reg::ra, 3);
  cpu.set_register(CPU::Reg::sp, 3);
  cpu.write_memory_word(cpu.get_pc(), instr);
  cpu.step();
  TEST(cpu.get_pc() == TEXT_START + 4, "BLT not taken (equal)");

  std::cout << "All BLT tests PASSED!" << std::endl;
  return 0;
}
