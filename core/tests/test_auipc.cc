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

  // AUIPC x4, 0x1000  (PC + 0x1000)
  uint32_t instr = make_u_type(rv32i::opcode::AUIPC, 4, 0x1000);
  cpu.write_memory_word(cpu.get_pc(), instr);
  cpu.step();
  TEST(cpu.registers_state()[4] == TEXT_START + 0x1000, "AUIPC x4, PC+0x1000");
  TEST(cpu.get_pc() == TEXT_START + 4, "PC advanced");

  // AUIPC x5, -0x1000 (PC - 0x1000)
  cpu.reset();
  instr = make_u_type(opcode::AUIPC, 5, -0x1000);
  cpu.write_memory_word(cpu.get_pc(), instr);
  cpu.step();
  TEST(cpu.registers_state()[5] == TEXT_START - 0x1000, "AUIPC x5, PC-0x1000");

  // AUIPC x0, 0x2000 (x0' yazma etkisiz)
  cpu.reset();
  instr = make_u_type(opcode::AUIPC, 0, 0x2000);
  cpu.write_memory_word(cpu.get_pc(), instr);
  cpu.step();
  TEST(cpu.registers_state()[0] == 0, "AUIPC x0 ignored");

  std::cout << "All AUIPC tests PASSED!" << std::endl;
  return 0;
}
