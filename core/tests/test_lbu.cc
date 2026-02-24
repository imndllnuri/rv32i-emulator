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
uint32_t make_load(uint32_t funct3, uint32_t rs1, uint32_t rd, int32_t imm) {
  uint32_t uimm = static_cast<uint32_t>(imm) & 0xFFF;
  return (uimm << 20) | (rs1 << 15) | (funct3 << 12) | (rd << 7) | 0x03;
}
} // namespace rv32i

int main() {
  using namespace rv32i;

  CPU cpu;
  cpu.reset();

  // LBU x3, 0(x1)   (funct3=4)
  uint32_t instr = make_load(0x4, 1, 3, 0);

  uint32_t data_addr = TEXT_START + 0x100;
  cpu.write_memory_byte(data_addr, 0x80);     // 128
  cpu.write_memory_byte(data_addr + 1, 0xFF); // 255
  cpu.write_memory_byte(data_addr + 2, 0x00); // 0

  // Test 1: LBU of 0x80 -> 0x80
  cpu.set_register(CPU::Reg::ra, data_addr);
  cpu.write_memory_word(cpu.get_pc(), instr);
  cpu.step();
  TEST(cpu.registers_state()[3] == 0x80, "LBU 0x80 -> 0x80");
  TEST(cpu.get_pc() == TEXT_START + 4, "PC advanced");
  TEST(cpu.registers_state()[0] == 0, "x0 unchanged");

  // Test 2: LBU of 0xFF -> 0xFF (not sign‑extended)
  cpu.reset();
  cpu.set_register(CPU::Reg::ra, data_addr + 1);
  cpu.write_memory_word(cpu.get_pc(), instr);
  cpu.step();
  TEST(cpu.registers_state()[3] == 0xFF, "LBU 0xFF -> 0xFF");

  // Test 3: LBU of 0x00 -> 0x00
  cpu.reset();
  cpu.set_register(CPU::Reg::ra, data_addr + 2);
  cpu.write_memory_word(cpu.get_pc(), instr);
  cpu.step();
  TEST(cpu.registers_state()[3] == 0x00, "LBU 0x00 -> 0x00");

  // Test 4: LBU with immediate offset (+1)
  cpu.reset();
  cpu.set_register(CPU::Reg::ra, data_addr);
  cpu.write_memory_word(cpu.get_pc(), make_load(0x4, 1, 3, 1));
  cpu.step();
  TEST(cpu.registers_state()[3] == 0xFF, "LBU with +1 offset reads 0xFF");

  std::cout << "All LBU tests PASSED!" << std::endl;
  return 0;
}
