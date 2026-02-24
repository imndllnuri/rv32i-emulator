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

  // SRL x3, x1, x2  (funct7=0, funct3=5, opcode=0x33)
  // Encoding: 0x50C1B3
  // uint32_t instr = 0x0020D1B3;
  uint32_t instr = 0x0020D1B3;
  CPU cpu;

  // -------- SRL --------
  cpu.reset();
  cpu.set_register(CPU::Reg::ra, 0x80000000);
  cpu.set_register(CPU::Reg::sp, 1);
  cpu.write_memory_word(cpu.get_pc(), instr);
  cpu.step();

  TEST(cpu.registers_state()[3] == 0x40000000,
       "SRL: 0x80000000 >> 1 = 0x40000000");

  // Test 1: logical right shift by 1
  cpu.reset();
  cpu.set_register(CPU::Reg::ra, 0x87654321);
  cpu.set_register(CPU::Reg::sp, 1);
  cpu.write_memory_word(cpu.get_pc(), instr);
  cpu.step();
  TEST(cpu.registers_state()[3] == (0x87654321 >> 1),
       "0x87654321 >> 1 (logical)");

  // Test 2: shift by 0 (unchanged)
  cpu.reset();
  cpu.set_register(CPU::Reg::ra, 0xDEADBEEF);
  cpu.set_register(CPU::Reg::sp, 0);
  cpu.write_memory_word(cpu.get_pc(), instr);
  cpu.step();
  TEST(cpu.registers_state()[3] == 0xDEADBEEF, "shift by 0 unchanged");

  // Test 3: shift by 31 (result becomes 0 or 1)
  cpu.reset();
  cpu.set_register(CPU::Reg::ra, 0x80000000); // high bit set
  cpu.set_register(CPU::Reg::sp, 31);
  cpu.write_memory_word(cpu.get_pc(), instr);
  cpu.step();
  TEST(cpu.registers_state()[3] == 1, "0x80000000 >> 31 = 1");

  // Test 4: shift amount only uses lower 5 bits
  cpu.reset();
  cpu.set_register(CPU::Reg::ra, 0x87654321);
  cpu.set_register(CPU::Reg::sp, 33); // mod 32 = 1
  cpu.write_memory_word(cpu.get_pc(), instr);
  cpu.step();
  TEST(cpu.registers_state()[3] == (0x87654321 >> 1),
       "shift by 33 = shift by 1");

  // Test 5: x0 unchanged
  cpu.reset();
  cpu.set_register(CPU::Reg::ra, 1);
  cpu.set_register(CPU::Reg::sp, 2);
  cpu.write_memory_word(cpu.get_pc(), instr);
  cpu.step();
  TEST(cpu.registers_state()[0] == 0, "x0 remains zero");

  std::cout << "All SRL tests PASSED!" << std::endl;
  return 0;
}
