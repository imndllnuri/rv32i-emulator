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

  // SLL x3, x1, x2  (funct7=0, funct3=1, opcode=0x33)
  // Encoding: 0x10C1B3
  uint32_t instr = 0x002091B3;

  CPU cpu;
  cpu.reset();

  // Test 1: shift left by 1
  cpu.set_register(CPU::Reg::ra, 0x12345678);
  cpu.set_register(CPU::Reg::sp, 1);
  cpu.write_memory_word(cpu.get_pc(), instr);
  cpu.step();
  TEST(cpu.registers_state()[3] == (0x12345678 << 1), "0x12345678 << 1");

  // Test 2: shift by 0 (unchanged)
  cpu.reset();
  cpu.set_register(CPU::Reg::ra, 0xDEADBEEF);
  cpu.set_register(CPU::Reg::sp, 0);
  cpu.write_memory_word(cpu.get_pc(), instr);
  cpu.step();
  TEST(cpu.registers_state()[3] == 0xDEADBEEF, "shift by 0 unchanged");

  // Test 3: shift by 31 (bits fall off)
  cpu.reset();
  cpu.set_register(CPU::Reg::ra, 1);
  cpu.set_register(CPU::Reg::sp, 31);
  cpu.write_memory_word(cpu.get_pc(), instr);
  cpu.step();
  TEST(cpu.registers_state()[3] == (1u << 31), "1 << 31 = 0x80000000");

  // Test 4: shift amount only uses lower 5 bits (shift by 32 -> shift by 0)
  cpu.reset();
  cpu.set_register(CPU::Reg::ra, 0x12345678);
  cpu.set_register(CPU::Reg::sp, 32); // 0x20, lower 5 bits = 0
  cpu.write_memory_word(cpu.get_pc(), instr);
  cpu.step();
  TEST(cpu.registers_state()[3] == 0x12345678,
       "shift by 32 (mod 32) = shift by 0");

  // Test 5: shift by 33 -> shift by 1
  cpu.reset();
  cpu.set_register(CPU::Reg::ra, 1);
  cpu.set_register(CPU::Reg::sp, 33);
  cpu.write_memory_word(cpu.get_pc(), instr);
  cpu.step();
  TEST(cpu.registers_state()[3] == 2, "shift by 33 (mod 32) = shift by 1");

  // Test 6: x0 unchanged
  cpu.reset();
  cpu.set_register(CPU::Reg::ra, 1);
  cpu.set_register(CPU::Reg::sp, 2);
  cpu.write_memory_word(cpu.get_pc(), instr);
  cpu.step();
  TEST(cpu.registers_state()[0] == 0, "x0 remains zero");

  std::cout << "All SLL tests PASSED!" << std::endl;
  return 0;
}
