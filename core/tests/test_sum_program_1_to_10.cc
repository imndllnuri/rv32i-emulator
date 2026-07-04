#include "../include/cpu.hpp"
#include "common/instr_encoders.hpp"
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


int main() {
  using namespace rv32i;

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
