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
  using namespace riscv;

  uint32_t instr = 0x4020D1B3;
  CPU cpu;

  // Test 1: arithmetic right shift of positive number (should be same as
  // logical)
  //

  cpu.reset();
  cpu.set_register(CPU::Reg::ra, 0x80000000);
  cpu.set_register(CPU::Reg::sp, 1);
  cpu.write_memory_word(cpu.get_pc(), instr);
  cpu.step();

  TEST(cpu.registers_state()[3] == 0xC0000000,
       "SRA: 0x80000000 >> 1 = 0xC0000000");

  cpu.reset();
  cpu.set_register(CPU::Reg::ra, 0x12345678);
  cpu.set_register(CPU::Reg::sp, 4);
  cpu.write_memory_word(cpu.get_pc(), instr);
  cpu.step();
  TEST(cpu.registers_state()[3] == (0x12345678 >> 4),
       "SRA positive = logical shift");

  // Test 2: arithmetic right shift of negative number (sign extends)
  cpu.reset();
  cpu.set_register(CPU::Reg::ra, 0x80000000); // -2147483648
  cpu.set_register(CPU::Reg::sp, 4);
  cpu.write_memory_word(cpu.get_pc(), instr);
  cpu.step();
  TEST(cpu.registers_state()[3] == 0xF8000000,
       "SRA of 0x80000000 by 4 = 0xF8000000");

  // Test 3: shift by 0 (unchanged)
  cpu.reset();
  cpu.set_register(CPU::Reg::ra, 0xDEADBEEF);
  cpu.set_register(CPU::Reg::sp, 0);
  cpu.write_memory_word(cpu.get_pc(), instr);
  cpu.step();
  TEST(cpu.registers_state()[3] == 0xDEADBEEF, "SRA by 0 unchanged");

  // Test 4: shift by 31 (only sign bit remains)
  cpu.reset();
  cpu.set_register(CPU::Reg::ra, 0x80000000);
  cpu.set_register(CPU::Reg::sp, 31);
  cpu.write_memory_word(cpu.get_pc(), instr);
  cpu.step();
  TEST(cpu.registers_state()[3] == 0xFFFFFFFF,
       "SRA of 0x80000000 by 31 = 0xFFFFFFFF");

  // Test 5: shift amount modulo 32
  cpu.reset();
  cpu.set_register(CPU::Reg::ra, 0x80000000);
  cpu.set_register(CPU::Reg::sp, 33); // 33 mod 32 = 1
  cpu.write_memory_word(cpu.get_pc(), instr);
  cpu.step();
  TEST(cpu.registers_state()[3] == 0xC0000000,
       "SRA by 33 = SRA by 1 (0xC0000000)");

  // Test 6: x0 unchanged
  cpu.reset();
  cpu.set_register(CPU::Reg::ra, 1);
  cpu.set_register(CPU::Reg::sp, 2);
  cpu.write_memory_word(cpu.get_pc(), instr);
  cpu.step();
  TEST(cpu.registers_state()[0] == 0, "x0 remains zero");

  std::cout << "All SRA tests PASSED!" << std::endl;
  return 0;
}
