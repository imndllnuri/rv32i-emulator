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

  // Test 1: SB x2, 0(x1)  (funct3=0) stores only the low byte of rs2
  cpu.write_memory_word(data_addr, 0xFFFFFFFF); // seed so other bytes are checkable
  cpu.set_register(CPU::Reg::ra, data_addr);
  cpu.set_register(CPU::Reg::sp, 0x000000AB);
  cpu.write_memory_word(cpu.get_pc(), make_s_type(0x0, 1, 2, 0));
  cpu.step();
  TEST(cpu.read_memory_byte(data_addr) == 0xAB, "SB writes low byte 0xAB");
  TEST((cpu.read_memory_word(data_addr) >> 8) == 0xFFFFFF,
       "SB leaves the other three bytes of the word untouched");
  TEST(cpu.get_pc() == TEXT_START + 4, "PC advanced");

  // Test 2: SB with positive offset, truncates a wider register value
  cpu.reset();
  cpu.set_register(CPU::Reg::ra, data_addr - 1);
  cpu.set_register(CPU::Reg::sp, 0xDEADBEEF); // only 0xEF should be stored
  cpu.write_memory_word(cpu.get_pc(), make_s_type(0x0, 1, 2, 1)); // offset +1
  cpu.step();
  TEST(cpu.read_memory_byte(data_addr) == 0xEF, "SB truncates to low byte 0xEF");

  std::cout << "All SB tests PASSED!" << std::endl;
  return 0;
}
