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

// Helper to build R-type instruction word
uint32_t make_r_type(uint32_t funct7, uint32_t rs2, uint32_t rs1,
                     uint32_t funct3, uint32_t rd) {
  return (funct7 << 25) | (rs2 << 20) | (rs1 << 15) | (funct3 << 12) |
         (rd << 7) | 0x33;
}

} // namespace riscv

int main() {
  using namespace riscv;

  CPU cpu;
  cpu.reset();

  // Set initial register values
  uint32_t x1_val = 0x12345678;
  uint32_t x2_val = 0x87654321;
  cpu.set_register(CPU::Reg::ra, x1_val); // x1
  cpu.set_register(CPU::Reg::sp, x2_val); // x2

  // Build instruction sequence (10 instructions)
  uint32_t code[] = {
      // add x3, x1, x2
      make_r_type(0x00, 2, 1, 0x0, 3),
      // sub x4, x3, x1   (x4 = x3 - x1)
      make_r_type(0x20, 1, 3, 0x0, 4), // note: rs2 = x1, rs1 = x3
      // and x5, x3, x4
      make_r_type(0x00, 4, 3, 0x7, 5),
      // or x6, x5, x2
      make_r_type(0x00, 2, 5, 0x6, 6),
      // xor x7, x6, x1
      make_r_type(0x00, 1, 6, 0x4, 7),
      // sll x8, x7, x2   (shift x7 by lower 5 bits of x2)
      make_r_type(0x00, 2, 7, 0x1, 8),
      // srl x9, x8, x1   (logical shift x8 by lower 5 bits of x1)
      make_r_type(0x00, 1, 8, 0x5, 9),
      // sra x10, x9, x3  (arithmetic shift x9 by lower 5 bits of x3)
      make_r_type(0x20, 3, 9, 0x5, 10),
      // slt x11, x10, x4
      make_r_type(0x00, 4, 10, 0x2, 11),
      // sltu x12, x11, x5
      make_r_type(0x00, 5, 11, 0x3, 12)};
  const int num_instr = sizeof(code) / sizeof(code[0]);

  // Write instructions to memory at TEXT_START
  for (int i = 0; i < num_instr; ++i) {
    cpu.write_memory_word(TEXT_START + i * 4, code[i]);
  }

  // Expected register values after each step (starting from x1, x2 known)
  uint32_t exp_x3, exp_x4, exp_x5, exp_x6, exp_x7, exp_x8, exp_x9, exp_x10,
      exp_x11, exp_x12;

  // Step 1: add x3, x1, x2
  exp_x3 = x1_val + x2_val; // 0x12345678 + 0x87654321 = 0x99999999
  TEST(cpu.step(), "step 1 ok");
  TEST(cpu.registers_state()[3] == exp_x3, "x3 after add");
  TEST(cpu.get_pc() == TEXT_START + 4, "PC after step 1");
  TEST(cpu.registers_state()[0] == 0, "x0 still zero");

  // Step 2: sub x4, x3, x1   (x4 = x3 - x1)
  exp_x4 = exp_x3 - x1_val;
  TEST(cpu.step(), "step 2 ok");
  TEST(cpu.registers_state()[4] == exp_x4, "x4 after sub");
  TEST(cpu.get_pc() == TEXT_START + 8, "PC after step 2");
  TEST(cpu.registers_state()[0] == 0, "x0 still zero");

  // Step 3: and x5, x3, x4
  exp_x5 = exp_x3 & exp_x4;

  exp_x5 = exp_x3 & exp_x4;
  TEST(cpu.step(), "step 3 ok");
  TEST(cpu.registers_state()[5] == exp_x5, "x5 after and");
  TEST(cpu.get_pc() == TEXT_START + 12, "PC after step 3");
  TEST(cpu.registers_state()[0] == 0, "x0 still zero");

  // Step 4: or x6, x5, x2
  exp_x6 = exp_x5 | x2_val;
  TEST(cpu.step(), "step 4 ok");
  TEST(cpu.registers_state()[6] == exp_x6, "x6 after or");
  TEST(cpu.get_pc() == TEXT_START + 16, "PC after step 4");
  TEST(cpu.registers_state()[0] == 0, "x0 still zero");

  // Step 5: xor x7, x6, x1
  exp_x7 = exp_x6 ^ x1_val;
  TEST(cpu.step(), "step 5 ok");
  TEST(cpu.registers_state()[7] == exp_x7, "x7 after xor");
  TEST(cpu.get_pc() == TEXT_START + 20, "PC after step 5");
  TEST(cpu.registers_state()[0] == 0, "x0 still zero");

  // Step 6: sll x8, x7, x2   (shift left by lower 5 bits of x2)
  uint32_t shift_x2 = x2_val & 0x1F; // x2 lower 5 bits = 0x21 & 0x1F = 0x01
  exp_x8 = exp_x7 << shift_x2;
  TEST(cpu.step(), "step 6 ok");
  TEST(cpu.registers_state()[8] == exp_x8, "x8 after sll");
  TEST(cpu.get_pc() == TEXT_START + 24, "PC after step 6");
  TEST(cpu.registers_state()[0] == 0, "x0 still zero");

  // Step 7: srl x9, x8, x1   (logical shift right by lower 5 bits of x1)
  uint32_t shift_x1 =
      x1_val & 0x1F; // x1 lower 5 bits = 0x78 & 0x1F = 0x18 (24)
  exp_x9 = exp_x8 >> shift_x1;
  TEST(cpu.step(), "step 7 ok");
  TEST(cpu.registers_state()[9] == exp_x9, "x9 after srl");
  TEST(cpu.get_pc() == TEXT_START + 28, "PC after step 7");
  TEST(cpu.registers_state()[0] == 0, "x0 still zero");

  // Step 8: sra x10, x9, x3   (arithmetic shift right by lower 5 bits of x3)
  uint32_t shift_x3 =
      exp_x3 & 0x1F; // x3 lower 5 bits = 0x99999999 & 0x1F = 0x19? Actually
                     // 0x99 = 153, 153 mod 32 = 25, so 0x19
  exp_x10 = static_cast<uint32_t>(static_cast<int32_t>(exp_x9) >>
                                  shift_x3); // arithmetic shift
  TEST(cpu.step(), "step 8 ok");
  TEST(cpu.registers_state()[10] == exp_x10, "x10 after sra");
  TEST(cpu.get_pc() == TEXT_START + 32, "PC after step 8");
  TEST(cpu.registers_state()[0] == 0, "x0 still zero");

  // Step 9: slt x11, x10, x4
  // signed comparison: (int32_t)exp_x10 < (int32_t)exp_x4
  exp_x11 =
      (static_cast<int32_t>(exp_x10) < static_cast<int32_t>(exp_x4)) ? 1 : 0;
  TEST(cpu.step(), "step 9 ok");
  TEST(cpu.registers_state()[11] == exp_x11, "x11 after slt");
  TEST(cpu.get_pc() == TEXT_START + 36, "PC after step 9");
  TEST(cpu.registers_state()[0] == 0, "x0 still zero");

  // Step 10: sltu x12, x11, x5
  exp_x12 = (exp_x11 < exp_x5) ? 1 : 0; // unsigned comparison
  TEST(cpu.step(), "step 10 ok");
  TEST(cpu.registers_state()[12] == exp_x12, "x12 after sltu");
  TEST(cpu.get_pc() == TEXT_START + 40, "PC after step 10");
  TEST(cpu.registers_state()[0] == 0, "x0 still zero");

  std::cout << "All multi-instruction R-type tests PASSED!" << std::endl;
  return 0;
}
