#include "../include/cpu.hpp"
#include <cstdint>
#include <iostream>
#include <vector>

#define TEST(cond, msg)                                                        \
  do {                                                                         \
    if (!(cond)) {                                                             \
      std::cerr << "FAIL: " << msg << " (" << __FILE__ << ":" << __LINE__      \
                << ")" << std::endl;                                           \
      return 1;                                                                \
    }                                                                          \
  } while (0)

namespace riscv {

// Helper'lar
uint32_t make_i_type(uint32_t funct3, uint32_t rs1, uint32_t rd, int32_t imm) {
  uint32_t uimm = static_cast<uint32_t>(imm) & 0xFFF;
  return (uimm << 20) | (rs1 << 15) | (funct3 << 12) | (rd << 7) | 0x13;
}

uint32_t make_r_type(uint32_t funct7, uint32_t rs2, uint32_t rs1,
                     uint32_t funct3, uint32_t rd) {
  return (funct7 << 25) | (rs2 << 20) | (rs1 << 15) | (funct3 << 12) |
         (rd << 7) | 0x33;
}

uint32_t make_b_type(uint32_t funct3, uint32_t rs1, uint32_t rs2, int32_t imm) {
  uint32_t uimm = static_cast<uint32_t>(imm);
  uint32_t imm12 = (uimm >> 12) & 1;
  uint32_t imm11 = (uimm >> 11) & 1;
  uint32_t imm10_5 = (uimm >> 5) & 0x3F;
  uint32_t imm4_1 = (uimm >> 1) & 0xF;
  return (imm12 << 31) | (imm10_5 << 25) | (rs2 << 20) | (rs1 << 15) |
         (funct3 << 12) | (imm4_1 << 8) | (imm11 << 7) | 0b1100011;
}

uint32_t make_ebreak() {
  return (1 << 20) | (0 << 15) | (0 << 12) | (0 << 7) | 0b1110011; // imm=1
}

} // namespace riscv

int main() {
  using namespace riscv;

  CPU cpu;
  cpu.reset();

  // add 1 to 10
  std::vector<uint32_t> code = {
      make_i_type(0b000, 0, 1, 1),       // addi x1, x0, 1
      make_i_type(0b000, 0, 2, 0),       // addi x2, x0, 0
      make_i_type(0b000, 0, 3, 11),      // addi x3, x0, 11
      make_r_type(0x00, 1, 2, 0b000, 2), // add x2, x2, x1
      make_i_type(0b000, 1, 1, 1),       // addi x1, x1, 1
      make_b_type(0b100, 1, 3, -12),     // blt x1, x3, loop (offset -12)
      make_ebreak()                      // ebreak
  };

  // load the program
  for (size_t i = 0; i < code.size(); ++i) {
    cpu.write_memory_word(TEXT_START + i * 4, code[i]);
  }

  // run the program
  cpu.run();

  // check results.
  uint32_t sum = cpu.registers_state()[2]; // x2
  TEST(sum == 55, "Toplam 55 olmalı (1+...+10)");
  std::cout << "Toplam = " << sum << std::endl;

  return 0;
}
