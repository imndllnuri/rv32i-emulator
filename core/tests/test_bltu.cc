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

namespace riscv {
uint32_t make_b_type(uint32_t funct3, uint32_t rs1, uint32_t rs2, int32_t imm) {
  uint32_t uimm = static_cast<uint32_t>(imm);
  uint32_t imm12 = (uimm >> 12) & 1;
  uint32_t imm11 = (uimm >> 11) & 1;
  uint32_t imm10_5 = (uimm >> 5) & 0x3F;
  uint32_t imm4_1 = (uimm >> 1) & 0xF;
  return (imm12 << 31) | (imm10_5 << 25) | (rs2 << 20) | (rs1 << 15) |
         (funct3 << 12) | (imm4_1 << 8) | (imm11 << 7) |
         0b1100011; // BRANCH opcode
}
} // namespace riscv

int main() {
  using namespace riscv;

  CPU cpu;
  cpu.reset();

  // BLTU x1, x2, +8   (funct3=6)
  uint32_t instr = make_b_type(0x6, 1, 2, 8);

  // Test 1: 3 < 5 (unsigned) -> taken
  cpu.set_register(CPU::Reg::ra, 3);
  cpu.set_register(CPU::Reg::sp, 5);
  cpu.write_memory_word(cpu.get_pc(), instr);
  cpu.step();
  TEST(cpu.get_pc() == TEXT_START + 8, "BLTU taken (3 < 5)");

  // Test 2: 5 < 3 false -> not taken
  cpu.reset();
  cpu.set_register(CPU::Reg::ra, 5);
  cpu.set_register(CPU::Reg::sp, 3);
  cpu.write_memory_word(cpu.get_pc(), instr);
  cpu.step();
  TEST(cpu.get_pc() == TEXT_START + 4, "BLTU not taken (5 >= 3)");

  // Test 3: unsigned: -1 (0xFFFFFFFF) büyüktür 1 -> 1 < -1? true (1 <
  // 0xFFFFFFFF)
  cpu.reset();
  cpu.set_register(CPU::Reg::ra, 1);
  cpu.set_register(CPU::Reg::sp, (uint32_t)-1);
  cpu.write_memory_word(cpu.get_pc(), instr);
  cpu.step();
  TEST(cpu.get_pc() == TEXT_START + 8, "BLTU taken (1 < 0xFFFFFFFF)");

  // Test 4: -1 < 1? false
  cpu.reset();
  cpu.set_register(CPU::Reg::ra, (uint32_t)-1);
  cpu.set_register(CPU::Reg::sp, 1);
  cpu.write_memory_word(cpu.get_pc(), instr);
  cpu.step();
  TEST(cpu.get_pc() == TEXT_START + 4, "BLTU not taken (0xFFFFFFFF >= 1)");

  // Test 5: eşit -> not taken
  cpu.reset();
  cpu.set_register(CPU::Reg::ra, 0x80000000);
  cpu.set_register(CPU::Reg::sp, 0x80000000);
  cpu.write_memory_word(cpu.get_pc(), instr);
  cpu.step();
  TEST(cpu.get_pc() == TEXT_START + 4, "BLTU not taken (equal)");

  std::cout << "All BLTU tests PASSED!" << std::endl;
  return 0;
}
