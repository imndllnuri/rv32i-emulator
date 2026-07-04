#include "../include/cpu.hpp"
#include "common/instr_encoders.hpp"
#include "../include/instruction.hpp"
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

  // LUI x3, 0x12345000  (imm = 0x12345 << 12)
  uint32_t instr = make_u_type(opcode::LUI, 3, 0x12345000);
  cpu.write_memory_word(cpu.get_pc(), instr);
  cpu.step();
  TEST(cpu.registers_state()[3] == 0x12345000, "LUI x3, 0x12345000");
  TEST(cpu.get_pc() == TEXT_START + 4, "PC advanced");
  TEST(cpu.registers_state()[0] == 0, "x0 unchanged");

  // LUI x0, 0xDEADB000 (x0' yazma etkisiz)
  cpu.reset();
  instr = make_u_type(opcode::LUI, 0, 0xDEADB000);
  cpu.write_memory_word(cpu.get_pc(), instr);
  cpu.step();
  TEST(cpu.registers_state()[0] == 0, "LUI x0 ignored");

  std::cout << "All LUI tests PASSED!" << std::endl;
  return 0;
}
