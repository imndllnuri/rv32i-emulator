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
// Helper: B‑tipi komut kelimesi oluştur
uint32_t make_b_type(uint32_t funct3, uint32_t rs1, uint32_t rs2, int32_t imm) {
  uint32_t uimm = static_cast<uint32_t>(imm);
  uint32_t imm12 = (uimm >> 12) & 1;
  uint32_t imm11 = (uimm >> 11) & 1;
  uint32_t imm10_5 = (uimm >> 5) & 0x3F;
  uint32_t imm4_1 = (uimm >> 1) & 0xF;
  return (imm12 << 31) | (imm10_5 << 25) | (rs2 << 20) | (rs1 << 15) |
         (funct3 << 12) | (imm4_1 << 8) | (imm11 << 7) |
         0b1100011; // BRANCH opcode
}
} // namespace riscv

int main() {
  using namespace riscv;

  CPU cpu;
  cpu.reset();

  // BEQ x1, x2, +8   (funct3=0)
  uint32_t instr = make_b_type(0x0, 1, 2, 8);

  // Test 1: Eşit değil – dallanma alınmaz
  cpu.set_register(CPU::Reg::ra, 10); // x1 = 10
  cpu.set_register(CPU::Reg::sp, 20); // x2 = 20
  cpu.write_memory_word(cpu.get_pc(), instr);
  cpu.step();
  TEST(cpu.get_pc() == TEXT_START + 4, "BEQ not taken, PC += 4");
  TEST(cpu.registers_state()[0] == 0, "x0 unchanged");

  // Test 2: Eşit – dallanma alınır (+8)
  cpu.reset();
  cpu.set_register(CPU::Reg::ra, 42);
  cpu.set_register(CPU::Reg::sp, 42);
  cpu.write_memory_word(cpu.get_pc(), instr);
  cpu.step();
  TEST(cpu.get_pc() == TEXT_START + 8, "BEQ taken, PC += 8");

  // Test 3: Negatif offset (–12)
  cpu.reset();
  cpu.set_register(CPU::Reg::ra, 100);
  cpu.set_register(CPU::Reg::sp, 100);
  cpu.write_memory_word(cpu.get_pc(), make_b_type(0x0, 1, 2, -12));
  cpu.step();
  TEST(cpu.get_pc() == TEXT_START - 12, "BEQ taken, PC += -12");

  // Test 4: Sıfır offset (döngü)
  cpu.reset();
  cpu.set_register(CPU::Reg::ra, 7);
  cpu.set_register(CPU::Reg::sp, 7);
  cpu.write_memory_word(cpu.get_pc(), make_b_type(0x0, 1, 2, 0));
  cpu.step();
  TEST(cpu.get_pc() == TEXT_START, "BEQ taken, PC unchanged (offset 0)");

  // Test 5: Kayıt değişmediğinden emin ol
  cpu.reset();
  cpu.set_register(CPU::Reg::ra, 5);
  cpu.set_register(CPU::Reg::sp, 5);
  cpu.set_register(CPU::Reg::gp, 0x12345678); // x3
  cpu.write_memory_word(cpu.get_pc(), instr);
  cpu.step();
  TEST(cpu.registers_state()[3] == 0x12345678, "x3 unchanged after BEQ");

  std::cout << "All BEQ tests PASSED!" << std::endl;
  return 0;
}
