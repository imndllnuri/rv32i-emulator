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

namespace rv32i {
uint32_t make_i_type(uint32_t funct3, uint32_t rs1, uint32_t rd, int32_t imm) {
  uint32_t uimm = static_cast<uint32_t>(imm) & 0xFFF;
  return (uimm << 20) | (rs1 << 15) | (funct3 << 12) | (rd << 7) | 0x13;
}
} // namespace rv32i

int main() {
  using namespace rv32i;

  // SLTIU x3, x1, 5   (funct3=3)
  uint32_t instr = make_i_type(0x3, 1, 3, 5);

  CPU cpu;
  cpu.reset();

  // Test 1: small < large
  cpu.set_register(CPU::Reg::ra, 3);
  cpu.write_memory_word(cpu.get_pc(), instr);
  cpu.step();
  TEST(cpu.registers_state()[3] == 1, "3 < 5 -> 1");

  // Test 2: large < small (false)
  cpu.reset();
  cpu.set_register(CPU::Reg::ra, 10);
  cpu.write_memory_word(cpu.get_pc(), instr);
  cpu.step();
  TEST(cpu.registers_state()[3] == 0, "10 < 5 -> 0");

  // Test 3: unsigned comparison: negative numbers are large
  cpu.reset();
  cpu.set_register(CPU::Reg::ra, 1);
  cpu.write_memory_word(cpu.get_pc(),
                        make_i_type(0x3, 1, 3, -1)); // imm = 0xFFF (4095)
  cpu.step();
  TEST(cpu.registers_state()[3] == 1, "1 < 4095 -> 1");

  // Test 4: 0xFFF < 1? false
  cpu.reset();
  cpu.set_register(
      CPU::Reg::ra,
      (uint32_t)-1); // 0xFFFFFFFF (larger than any 12-bit immediate)
  cpu.write_memory_word(cpu.get_pc(), make_i_type(0x3, 1, 3, 1));
  cpu.step();
  TEST(cpu.registers_state()[3] == 0, "0xFFFFFFFF < 1 -> 0");

  // Test 5: equal values
  cpu.reset();
  cpu.set_register(CPU::Reg::ra, 0x80000000);
  cpu.write_memory_word(cpu.get_pc(),
                        make_i_type(0x3, 1, 3, 0x80000000 & 0xFFF));
  cpu.step();
  TEST(cpu.registers_state()[3] == 0, "equal values -> 0");

  // Test 6: x0 unchanged
  cpu.reset();
  cpu.set_register(CPU::Reg::ra, 1);
  cpu.write_memory_word(cpu.get_pc(), instr);
  cpu.step();
  TEST(cpu.registers_state()[0] == 0, "x0 remains zero");

  std::cout << "All SLTIU tests PASSED!" << std::endl;
  return 0;
}
