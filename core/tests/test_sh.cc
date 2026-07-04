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

  uint32_t data_addr = TEXT_START + 0x100;

  // Test 1: SH x2, 0(x1)  (funct3=1) stores only the low 16 bits of rs2
  cpu.write_memory_word(data_addr, 0xFFFFFFFF); // seed so upper bytes are checkable
  cpu.set_register(CPU::Reg::ra, data_addr);
  cpu.set_register(CPU::Reg::sp, 0xBEEFCAFE);
  cpu.write_memory_word(cpu.get_pc(), make_s_type(0x1, 1, 2, 0));
  cpu.step();
  TEST(cpu.read_memory_half(data_addr) == 0xCAFE, "SH writes low half 0xCAFE");
  TEST((cpu.read_memory_word(data_addr) >> 16) == 0xFFFF,
       "SH leaves the upper half of the word untouched");
  TEST(cpu.get_pc() == TEXT_START + 4, "PC advanced");

  // Test 2: SH with positive offset
  cpu.reset();
  cpu.set_register(CPU::Reg::ra, data_addr - 2);
  cpu.set_register(CPU::Reg::sp, 0x1234ABCD);
  cpu.write_memory_word(cpu.get_pc(), make_s_type(0x1, 1, 2, 2)); // offset +2
  cpu.step();
  TEST(cpu.read_memory_half(data_addr) == 0xABCD, "SH with offset +2");

  std::cout << "All SH tests PASSED!" << std::endl;
  return 0;
}
