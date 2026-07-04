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

  // BNE x1, x2, +8   (funct3=1)
  uint32_t instr = make_b_type(0x1, 1, 2, 8);

  // Test 1: equal – not taken
  cpu.set_register(CPU::Reg::ra, 10);
  cpu.set_register(CPU::Reg::sp, 10);
  cpu.write_memory_word(cpu.get_pc(), instr);
  cpu.step();
  TEST(cpu.get_pc() == TEXT_START + 4, "BNE not taken (equal), PC += 4");

  // Test 2: its bne so taken because not equal
  cpu.reset();
  cpu.set_register(CPU::Reg::ra, 10);
  cpu.set_register(CPU::Reg::sp, 20);
  cpu.write_memory_word(cpu.get_pc(), instr);
  cpu.step();
  TEST(cpu.get_pc() == TEXT_START + 8, "BNE taken, PC += 8");

  // Test 3: Negative offset
  cpu.reset();
  cpu.set_register(CPU::Reg::ra, 1);
  cpu.set_register(CPU::Reg::sp, 2);
  cpu.write_memory_word(cpu.get_pc(), make_b_type(0x1, 1, 2, -12));
  cpu.step();
  TEST(cpu.get_pc() == TEXT_START - 12, "BNE taken with negative offset");

  // Test 4: x0 constant
  cpu.reset();
  cpu.set_register(CPU::Reg::ra, 5);
  cpu.set_register(CPU::Reg::sp, 6);
  cpu.write_memory_word(cpu.get_pc(), instr);
  cpu.step();
  TEST(cpu.registers_state()[0] == 0, "x0 unchanged");

  std::cout << "All BNE tests PASSED!" << std::endl;
  return 0;
}
