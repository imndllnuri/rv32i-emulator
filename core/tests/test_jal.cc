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
// creates j-type jal instruction (imm 20-bit, signed)
uint32_t make_jal(uint32_t rd, int32_t imm) {
  uint32_t uimm = static_cast<uint32_t>(imm);
  uint32_t imm20 = (uimm >> 20) & 1;
  uint32_t imm19_12 = (uimm >> 12) & 0xFF;
  uint32_t imm11 = (uimm >> 11) & 1;
  uint32_t imm10_1 = (uimm >> 1) & 0x3FF;
  return (imm20 << 31) | (imm10_1 << 21) | (imm11 << 20) | (imm19_12 << 12) |
         (rd << 7) | 0b1101111; // JAL opcode
}
} // namespace rv32i

int main() {
  using namespace rv32i;

  CPU cpu;
  cpu.reset();

  // Test 1: JAL x1, +8
  uint32_t instr = make_jal(1, 8);
  cpu.write_memory_word(cpu.get_pc(), instr);
  cpu.step();
  TEST(cpu.get_pc() == TEXT_START + 8, "JAL +8, PC doğru");
  TEST(cpu.registers_state()[1] == TEXT_START + 4, "x1 = PC+4 (link)");
  TEST(cpu.registers_state()[0] == 0, "x0 unchanged");

  // Test 2: JAL x0, +16  (rd = x0)
  cpu.reset();
  instr = make_jal(0, 16);
  cpu.write_memory_word(cpu.get_pc(), instr);
  cpu.step();
  TEST(cpu.get_pc() == TEXT_START + 16, "JAL x0, PC += 16");
  TEST(cpu.registers_state()[0] == 0, "x0 hala 0");

  // Test 3: JAL x2, -12  (backward jump)
  cpu.reset();
  instr = make_jal(2, -12);
  cpu.write_memory_word(cpu.get_pc(), instr);
  cpu.step();
  TEST(cpu.get_pc() == TEXT_START - 12, "JAL -12, PC doğru");
  TEST(cpu.registers_state()[2] == TEXT_START + 4, "x2 = PC+4");

  // Test 4: JAL x3, 0  (offset zero – infinite loop)
  cpu.reset();
  instr = make_jal(3, 0);
  cpu.write_memory_word(cpu.get_pc(), instr);
  cpu.step();
  TEST(cpu.get_pc() == TEXT_START, "JAL 0, PC sabit");
  TEST(cpu.registers_state()[3] == TEXT_START + 4, "x3 = PC+4");

  // Test 5: register values should stay same
  cpu.reset();
  cpu.set_register(CPU::Reg::gp, 0x12345678); // x3
  instr = make_jal(4, 20);
  cpu.write_memory_word(cpu.get_pc(), instr);
  cpu.step();
  TEST(cpu.registers_state()[3] == 0x12345678, "x3 unchanged after JAL");
  TEST(cpu.registers_state()[4] == TEXT_START + 4, "x4 link doğru");

  std::cout << "All JAL tests PASSED!" << std::endl;
  return 0;
}
