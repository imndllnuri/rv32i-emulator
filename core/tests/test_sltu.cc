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

  // SLTU x3, x1, x2  (funct7=0, funct3=3, opcode=0x33)
  // Encoding: 0x30C1B3
  uint32_t instr = 0x0020B1B3;

  CPU cpu;
  cpu.reset();

  // Test 1: small < large
  cpu.set_register(CPU::Reg::ra, 3);
  cpu.set_register(CPU::Reg::sp, 5);
  cpu.write_memory_word(cpu.get_pc(), instr);
  cpu.step();
  TEST(cpu.registers_state()[3] == 1, "3 < 5 -> 1");

  // Test 2: large < small (false)
  cpu.reset();
  cpu.set_register(CPU::Reg::ra, 10);
  cpu.set_register(CPU::Reg::sp, 2);
  cpu.write_memory_word(cpu.get_pc(), instr);
  cpu.step();
  TEST(cpu.registers_state()[3] == 0, "10 < 2 -> 0");

  // Test 3: unsigned comparison: negative numbers are large
  cpu.reset();
  cpu.set_register(CPU::Reg::ra, 1);
  cpu.set_register(CPU::Reg::sp, (uint32_t)-1); // 0xFFFFFFFF (largest unsigned)
  cpu.write_memory_word(cpu.get_pc(), instr);
  cpu.step();
  TEST(cpu.registers_state()[3] == 1, "1 < 0xFFFFFFFF -> 1");

  // Test 4: 0xFFFFFFFF < 1? false
  cpu.reset();
  cpu.set_register(CPU::Reg::ra, (uint32_t)-1);
  cpu.set_register(CPU::Reg::sp, 1);
  cpu.write_memory_word(cpu.get_pc(), instr);
  cpu.step();
  TEST(cpu.registers_state()[3] == 0, "0xFFFFFFFF < 1 -> 0");

  // Test 5: equal values
  cpu.reset();
  cpu.set_register(CPU::Reg::ra, 0x80000000);
  cpu.set_register(CPU::Reg::sp, 0x80000000);
  cpu.write_memory_word(cpu.get_pc(), instr);
  cpu.step();
  TEST(cpu.registers_state()[3] == 0, "0x80000000 < 0x80000000 -> 0");

  // Test 6: x0 unchanged
  cpu.reset();
  cpu.set_register(CPU::Reg::ra, 1);
  cpu.set_register(CPU::Reg::sp, 2);
  cpu.write_memory_word(cpu.get_pc(), instr);
  cpu.step();
  TEST(cpu.registers_state()[0] == 0, "x0 remains zero");

  std::cout << "All SLTU tests PASSED!" << std::endl;
  return 0;
}
