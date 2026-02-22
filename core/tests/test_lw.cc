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
uint32_t make_load(uint32_t funct3, uint32_t rs1, uint32_t rd, int32_t imm) {
  uint32_t uimm = static_cast<uint32_t>(imm) & 0xFFF;
  return (uimm << 20) | (rs1 << 15) | (funct3 << 12) | (rd << 7) | 0x03;
}
} // namespace riscv

int main() {
  using namespace riscv;

  CPU cpu;
  cpu.reset();

  // LW x3, 0(x1)   (funct3=2)
  uint32_t instr = make_load(0x2, 1, 3, 0);

  uint32_t data_addr = TEXT_START + 0x100;
  uint32_t test_word = 0xDEADBEEF;
  cpu.write_memory_word(data_addr, test_word);

  // Test 1: LW from address in x1
  cpu.set_register(CPU::Reg::ra, data_addr);
  cpu.write_memory_word(cpu.get_pc(), instr);
  cpu.step();
  TEST(cpu.registers_state()[3] == test_word, "LW reads 0xDEADBEEF");
  TEST(cpu.get_pc() == TEXT_START + 4, "PC advanced");
  TEST(cpu.registers_state()[0] == 0, "x0 unchanged");

  // Test 2: LW with positive offset
  cpu.reset();
  cpu.set_register(CPU::Reg::ra, data_addr - 4);
  cpu.write_memory_word(cpu.get_pc(), make_load(0x2, 1, 3, 4)); // offset +4
  cpu.step();
  TEST(cpu.registers_state()[3] == test_word, "LW with offset +4");

  // Test 3: LW unaligned (should work)
  cpu.reset();
  // Write a known word at data_addr+1 (bytes: 0x11,0x22,0x33,0x44)
  cpu.write_memory_byte(data_addr + 1, 0x11);
  cpu.write_memory_byte(data_addr + 2, 0x22);
  cpu.write_memory_byte(data_addr + 3, 0x33);
  cpu.write_memory_byte(data_addr + 4, 0x44);
  cpu.set_register(CPU::Reg::ra, data_addr + 1);
  cpu.write_memory_word(cpu.get_pc(), instr);
  cpu.step();
  uint32_t expected = 0x44332211; // little‑endian
  TEST(cpu.registers_state()[3] == expected, "Unaligned LW reads 0x44332211");

  std::cout << "All LW tests PASSED!" << std::endl;
  return 0;
}
