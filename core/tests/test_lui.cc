#include "../include/cpu.hpp"
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

namespace rv32i {
uint32_t make_u_type(uint32_t opcode, uint32_t rd, int32_t imm) {
  uint32_t uimm = static_cast<uint32_t>(imm) & 0xFFFFF000; // sadece üst 20 bit
  return (uimm) | (rd << 7) | opcode;
}
} // namespace rv32i

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
