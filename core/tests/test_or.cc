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

  // OR x3, x1, x2  (funct7=0, funct3=6, opcode=0x33)
  // Encoding: 0x60C1B3  (funct7=0, rs2=2, rs1=1, funct3=6, rd=3, opcode=0x33)
  uint32_t instr = 0x60C1B3;

  CPU cpu;
  cpu.reset();

  // Test 1: basic OR
  cpu.set_register(CPU::Reg::ra, 0b1010);
  cpu.set_register(CPU::Reg::sp, 0b1100);
  cpu.write_memory_word(cpu.get_pc(), instr);
  cpu.step();
  TEST(cpu.registers_state()[3] == (0b1010 | 0b1100),
       "0b1010 | 0b1100 = 0b1110");

  // Test 2: OR with zero (unchanged)
  cpu.reset();
  cpu.set_register(CPU::Reg::ra, 0xDEADBEEF);
  cpu.set_register(CPU::Reg::sp, 0);
  cpu.write_memory_word(cpu.get_pc(), instr);
  cpu.step();
  TEST(cpu.registers_state()[3] == 0xDEADBEEF, "OR with zero -> unchanged");

  // Test 3: OR with all ones (all ones)
  cpu.reset();
  cpu.set_register(CPU::Reg::ra, 0x12345678);
  cpu.set_register(CPU::Reg::sp, 0xFFFFFFFF);
  cpu.write_memory_word(cpu.get_pc(), instr);
  cpu.step();
  TEST(cpu.registers_state()[3] == 0xFFFFFFFF, "OR with all ones -> all ones");

  // Test 4: OR with itself (same)
  cpu.reset();
  cpu.set_register(CPU::Reg::ra, 0xA5A5A5A5);
  cpu.set_register(CPU::Reg::sp, 0xA5A5A5A5);
  cpu.write_memory_word(cpu.get_pc(), instr);
  cpu.step();
  TEST(cpu.registers_state()[3] == 0xA5A5A5A5, "OR with itself -> unchanged");

  // Test 5: x0 unchanged
  cpu.reset();
  cpu.set_register(CPU::Reg::ra, 1);
  cpu.set_register(CPU::Reg::sp, 2);
  cpu.write_memory_word(cpu.get_pc(), instr);
  cpu.step();
  TEST(cpu.registers_state()[0] == 0, "x0 remains zero");

  std::cout << "All OR tests PASSED!" << std::endl;
  return 0;
}
