// TEST: TEST PASSED FOR ADD INSTRUCTION
#include "../include/cpu.hpp"
#include <cstdint>
#include <iostream>

// Simple test assertion with message
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

  CPU cpu;
  cpu.reset();

  cpu.set_register(CPU::Reg::ra, 5); // x1 = 5
  cpu.set_register(CPU::Reg::sp, 3); // x2 = 3

  uint32_t instr = 0x002081B3;
  cpu.write_memory_word(cpu.get_pc(), instr);

  bool step_ok = cpu.step();
  TEST(step_ok, "step() returned true");

  TEST(cpu.registers_state()[3] == 8, "x3 == 8");
  TEST(cpu.get_pc() == TEXT_START + 4, "PC advanced by 4");

  TEST(cpu.registers_state()[0] == 0, "x0 == 0");

  cpu.reset();
  cpu.set_register(CPU::Reg::ra, 0xFFFFFFFF); // -1
  cpu.set_register(CPU::Reg::sp, 1);
  cpu.write_memory_word(cpu.get_pc(), instr);
  cpu.step();
  TEST(cpu.registers_state()[3] == 0, "-1 + 1 == 0");

  cpu.reset();
  cpu.set_register(CPU::Reg::ra, 0x80000000); // -2147483648
  cpu.set_register(CPU::Reg::sp, 0x80000000);
  cpu.write_memory_word(cpu.get_pc(), instr);
  cpu.step();
  TEST(cpu.registers_state()[3] == 0, "0x80000000 + 0x80000000 wraps to 0");

  std::cout << "All tests PASSED!" << std::endl;
  return 0;
}
