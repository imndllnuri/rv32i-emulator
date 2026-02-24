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

  // SUB x3, x1, x2  (funct7=0x20, funct3=0, opcode=0x33)
  // Encoding: 0x402081B3
  uint32_t instr = 0x402081B3;

  // -------------------- Test 1: basic subtraction --------------------
  CPU cpu;
  cpu.reset();
  cpu.set_register(CPU::Reg::ra, 10); // x1 = 10
  cpu.set_register(CPU::Reg::sp, 3);  // x2 = 3
  cpu.write_memory_word(cpu.get_pc(), instr);
  cpu.step();
  TEST(cpu.registers_state()[3] == 7, "10 - 3 = 7");

  // -------------------- Test 2: negative result --------------------
  cpu.reset();
  cpu.set_register(CPU::Reg::ra, 3);
  cpu.set_register(CPU::Reg::sp, 10);
  cpu.write_memory_word(cpu.get_pc(), instr);
  cpu.step();
  TEST(cpu.registers_state()[3] == (uint32_t)(-7),
       "3 - 10 = -7 (wraps to 0xFFFFFFF9)");

  // -------------------- Test 3: subtract zero --------------------
  cpu.reset();
  cpu.set_register(CPU::Reg::ra, 42);
  cpu.set_register(CPU::Reg::sp, 0);
  cpu.write_memory_word(cpu.get_pc(), instr);
  cpu.step();
  TEST(cpu.registers_state()[3] == 42, "42 - 0 = 42");

  // -------------------- Test 4: subtract from zero --------------------
  cpu.reset();
  cpu.set_register(CPU::Reg::ra, 0);
  cpu.set_register(CPU::Reg::sp, 5);
  cpu.write_memory_word(cpu.get_pc(), instr);
  cpu.step();
  TEST(cpu.registers_state()[3] == (uint32_t)(-5), "0 - 5 = -5");

  // -------------------- Test 5: overflow (wrap) --------------------
  cpu.reset();
  cpu.set_register(CPU::Reg::ra, 0x80000000); // -2147483648
  cpu.set_register(CPU::Reg::sp, 1);
  cpu.write_memory_word(cpu.get_pc(), instr);
  cpu.step();
  TEST(cpu.registers_state()[3] == 0x7FFFFFFF,
       "0x80000000 - 1 = 0x7FFFFFFF (overflow)");

  // -------------------- Test 6: x0 unchanged --------------------
  cpu.reset();
  cpu.set_register(CPU::Reg::ra, 5);
  cpu.set_register(CPU::Reg::sp, 2);
  cpu.write_memory_word(cpu.get_pc(), instr);
  cpu.step();
  TEST(cpu.registers_state()[0] == 0, "x0 remains zero");

  std::cout << "All SUB tests PASSED!" << std::endl;
  return 0;
}
