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

  // Test 1: SW x2, 0(x1)  (funct3=2)  stores rs2 at address in x1
  cpu.set_register(CPU::Reg::ra, data_addr);
  cpu.set_register(CPU::Reg::sp, 0xDEADBEEF);
  cpu.write_memory_word(cpu.get_pc(), make_s_type(0x2, 1, 2, 0));
  cpu.step();
  TEST(cpu.read_memory_word(data_addr) == 0xDEADBEEF, "SW writes 0xDEADBEEF");
  TEST(cpu.get_pc() == TEXT_START + 4, "PC advanced");

  // Test 2: SW with positive offset
  cpu.reset();
  cpu.set_register(CPU::Reg::ra, data_addr - 4);
  cpu.set_register(CPU::Reg::sp, 0x12345678);
  cpu.write_memory_word(cpu.get_pc(), make_s_type(0x2, 1, 2, 4)); // offset +4
  cpu.step();
  TEST(cpu.read_memory_word(data_addr) == 0x12345678, "SW with offset +4");

  // Test 3: SW with negative offset
  cpu.reset();
  cpu.set_register(CPU::Reg::ra, data_addr + 4);
  cpu.set_register(CPU::Reg::sp, 0xCAFEF00D);
  cpu.write_memory_word(cpu.get_pc(), make_s_type(0x2, 1, 2, -4));
  cpu.step();
  TEST(cpu.read_memory_word(data_addr) == 0xCAFEF00D, "SW with offset -4");

  // Test 4: x0 as source stores zero regardless of x0's (ignored) writes
  cpu.reset();
  cpu.write_memory_word(data_addr, 0xFFFFFFFF); // seed with non-zero
  cpu.set_register(CPU::Reg::ra, data_addr);
  cpu.write_memory_word(cpu.get_pc(), make_s_type(0x2, 1, 0, 0)); // rs2=x0
  cpu.step();
  TEST(cpu.read_memory_word(data_addr) == 0, "SW x0 stores zero");

  std::cout << "All SW tests PASSED!" << std::endl;
  return 0;
}
