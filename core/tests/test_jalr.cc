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
// creates j-type instruction (funct4=0, opcode=JALR)
uint32_t make_jalr(uint32_t rs1, uint32_t rd, int32_t imm) {
  uint32_t uimm = static_cast<uint32_t>(imm) & 0xFFF;
  return (uimm << 20) | (rs1 << 15) | (0x0 << 12) | (rd << 7) |
         0b1100111; // JALR opcode
}
} // namespace rv32i

int main() {
  using namespace rv32i;

  CPU cpu;
  cpu.reset();

  uint32_t base_addr = TEXT_START + 0x100; // test address

  // Test 1: JALR x1, x2, +8  (x2 = base_addr, hedef = base_addr+8)
  cpu.set_register(CPU::Reg::sp, base_addr); // x2
  uint32_t instr = make_jalr(2, 1, 8);
  cpu.write_memory_word(cpu.get_pc(), instr);
  cpu.step();
  TEST(cpu.get_pc() == (base_addr + 8), "JALR +8, PC doğru");
  TEST(cpu.registers_state()[1] == TEXT_START + 4, "x1 = PC+4 (link)");
  TEST(cpu.registers_state()[0] == 0, "x0 unchanged");

  // Test 2: JALR x0, x3, -12 (rd = x0)
  cpu.reset();
  cpu.set_register(CPU::Reg::gp, base_addr); // x3
  instr = make_jalr(3, 0, -12);
  cpu.write_memory_word(cpu.get_pc(), instr);
  cpu.step();
  TEST(cpu.get_pc() == (base_addr - 12), "JALR -12, PC doğru");
  TEST(cpu.registers_state()[0] == 0, "x0 hala 0");

  // Test 3: JALR x4, x5, 0  (target = rs1 + 0)
  cpu.reset();
  cpu.set_register(CPU::Reg::t0, 0x2000); // x5
  instr = make_jalr(5, 4, 0);
  cpu.write_memory_word(cpu.get_pc(), instr);
  cpu.step();
  TEST(cpu.get_pc() == 0x2000, "JALR 0, PC = rs1");
  TEST(cpu.registers_state()[4] == TEXT_START + 4, "x4 link doğru");

  // Test 4: target address and JALR lowest bit should be zeroed
  cpu.reset();
  cpu.set_register(CPU::Reg::a0, 0x1003); // x10 (tek adres)
  instr = make_jalr(10, 6, 0);
  cpu.write_memory_word(cpu.get_pc(), instr);
  cpu.step();
  TEST(cpu.get_pc() == 0x1002, "JALR LSB cleared: 0x1003 -> 0x1002");
  TEST(cpu.registers_state()[6] == TEXT_START + 4, "x6 link doğru");

  // Test 5: JALR with negative imm and LSB clearing
  cpu.reset();
  cpu.set_register(CPU::Reg::a1, 0x2001); // x11
  instr = make_jalr(11, 7, -5);           // -5 = 0xFFF... (0xFFB)
  cpu.write_memory_word(cpu.get_pc(), instr);
  cpu.step();
  // 0x2001 + (-5) = 0x1FFC, LSB alread 0
  TEST(cpu.get_pc() == 0x1FFC, "JALR -5, PC = rs1 + imm");
  TEST(cpu.registers_state()[7] == TEXT_START + 4, "x7 link doğru");

  // Test 6: register values should stay same
  cpu.reset();
  cpu.set_register(CPU::Reg::a2, 0x3000);     // x12
  cpu.set_register(CPU::Reg::a3, 0xDEADBEEF); // x13
  instr = make_jalr(12, 8, 4);
  cpu.write_memory_word(cpu.get_pc(), instr);
  cpu.step();
  TEST(cpu.registers_state()[13] == 0xDEADBEEF, "x13 unchanged");
  TEST(cpu.registers_state()[8] == TEXT_START + 4, "x8 link doğru");

  std::cout << "All JALR tests PASSED!" << std::endl;
  return 0;
}
