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

// Helper to build an R‑type instruction word
uint32_t make_r_type(uint32_t funct7, uint32_t rs2, uint32_t rs1,
                     uint32_t funct3, uint32_t rd) {
  return (funct7 << 25) | (rs2 << 20) | (rs1 << 15) | (funct3 << 12) |
         (rd << 7) | 0x33;
}

} // namespace rv32i

int main() {
  using namespace rv32i;

  CPU cpu;
  cpu.reset();

  // Set initial register values
  uint32_t x1_val = 0x12345678;
  uint32_t x2_val = 0x87654321;
  cpu.set_register(CPU::Reg::ra, x1_val); // x1
  cpu.set_register(CPU::Reg::sp, x2_val); // x2

  // Program: three R‑type instructions followed by EBREAK
  uint32_t code[] = {// add x3, x1, x2
                     make_r_type(0x00, 2, 1, 0x0, 3),
                     // sub x4, x3, x1   (x4 = x3 - x1)
                     make_r_type(0x20, 1, 3, 0x0, 4),
                     // and x5, x3, x4
                     make_r_type(0x00, 4, 3, 0x7, 5),
                     // EBREAK (environment break)
                     0x00100073};
  const int num_instr = sizeof(code) / sizeof(code[0]);

  // Write the program into memory at TEXT_START
  for (int i = 0; i < num_instr; ++i) {
    cpu.write_memory_word(TEXT_START + i * 4, code[i]);
  }

  // Run the whole program – it will stop when EBREAK raises an exception
  cpu.run();

  // Expected results (computed using ordinary C++ operations)
  uint32_t x3 = x1_val + x2_val; // 0x99999999
  uint32_t x4 = x3 - x1_val;     // = x2_val (0x87654321)
  uint32_t x5 = x3 & x4;         // 0x99999999 & 0x87654321

  // Verify final register values
  TEST(cpu.registers_state()[3] == x3, "x3 after run");
  TEST(cpu.registers_state()[4] == x4, "x4 after run");
  TEST(cpu.registers_state()[5] == x5, "x5 after run");
  // PC should point to the EBREAK instruction
  TEST(cpu.get_pc() == TEXT_START + 3 * 4, "PC at EBREAK");
  // x0 must remain zero
  TEST(cpu.registers_state()[0] == 0, "x0 unchanged");

  std::cout << "run() with multiple R‑type instructions PASSED!" << std::endl;
  return 0;
}
