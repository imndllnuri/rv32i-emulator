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

  // 1. Create CPU
  CPU cpu;
  cpu.reset();

  // 2. XOR instruction: XOR x3, x1, x2
  // Encoding: 0x20C1B3 (funct7=0, rs2=2, rs1=1, funct3=4, rd=3, opcode=0x33)
  uint32_t instr = 0x20C1B3;

  // -------------------- Test 1: basic XOR --------------------
  cpu.reset();
  cpu.set_register(CPU::Reg::ra, 0x12345678); // x1
  cpu.set_register(CPU::Reg::sp, 0xFFFFFFFF); // x2 (all ones)
  cpu.write_memory_word(cpu.get_pc(), instr);
  cpu.step();
  TEST(cpu.registers_state()[3] == (0x12345678 ^ 0xFFFFFFFF),
       "XOR with all ones");

  // -------------------- Test 2: XOR with itself (should be 0)
  // --------------------
  cpu.reset();
  cpu.set_register(CPU::Reg::ra, 0xA5A5A5A5); // x1
  cpu.set_register(CPU::Reg::sp, 0xA5A5A5A5); // x2 (same)
  cpu.write_memory_word(cpu.get_pc(), instr);
  cpu.step();
  TEST(cpu.registers_state()[3] == 0, "XOR same value -> 0");

  // -------------------- Test 3: XOR with zero (should be same)
  // --------------------
  cpu.reset();
  cpu.set_register(CPU::Reg::ra, 0xDEADBEEF); // x1
  cpu.set_register(CPU::Reg::sp, 0x00000000); // x2 = 0
  cpu.write_memory_word(cpu.get_pc(), instr);
  cpu.step();
  TEST(cpu.registers_state()[3] == 0xDEADBEEF, "XOR with zero -> unchanged");

  // -------------------- Test 4: XOR with alternating bits --------------------
  cpu.reset();
  cpu.set_register(CPU::Reg::ra,
                   0b10101010101010101010101010101010); // 0xAAAAAAAA
  cpu.set_register(CPU::Reg::sp,
                   0b01010101010101010101010101010101); // 0x55555555
  cpu.write_memory_word(cpu.get_pc(), instr);
  cpu.step();
  TEST(cpu.registers_state()[3] == 0xFFFFFFFF, "XOR complements -> all ones");

  // -------------------- Test 5: x0 remains zero --------------------
  cpu.reset();
  cpu.set_register(CPU::Reg::ra, 0x12345678);
  cpu.set_register(CPU::Reg::sp, 0x87654321);
  cpu.write_memory_word(cpu.get_pc(), instr);
  cpu.step();
  TEST(cpu.registers_state()[0] == 0, "x0 unchanged");

  std::cout << "All XOR tests PASSED!" << std::endl;
  return 0;
}
