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

int main() {
  using namespace riscv;

  // SRA x3, x1, x2  (funct7=0x20, funct3=5, opcode=0x33)
  // Encoding: 0x40D0C1B3? Let's compute: funct7=0x20 (bits 31-25), rs2=2,
  // rs1=1, funct3=5, rd=3, opcode=0x33 Bits: 0100000 00010 00001 101 00011
  // 0110011 -> 0x4020C1B3? Wait, need correct hex. funct7=0100000, rs2=00010,
  // rs1=00001, funct3=101, rd=00011, opcode=0110011 Group: 0100000 00010 00001
  // 101 00011 0110011 = 01000000001000001101000110110011 = 0x4020C1B3? Let's
  // compute in pieces: 0100000 00010 00001 101 00011 0110011 = 0x40 (bits
  // 31-26?), better to compute as: (0x20 << 25) | (2 << 20) | (1 << 15) | (5 <<
  // 12) | (3 << 7) | 0x33 = (0x20 << 25) = 0x40000000
  // + (2 << 20) = 0x00200000
  // + (1 << 15) = 0x00008000
  // + (5 << 12) = 0x00005000? 5<<12 = 0x5000
  // + (3 << 7)  = 0x00000180? 3<<7 = 0x180
  // + 0x33 = 0x33
  // Sum: 0x40000000 + 0x00200000 = 0x40200000
  // + 0x00008000 = 0x40208000
  // + 0x00005000 = 0x4020D000? Wait 0x40208000+0x5000 = 0x4020D000, but that
  // seems off. Actually 0x5000 is 5<<12, yes. Then +0x180 gives 0x4020D180,
  // then +0x33 = 0x4020D1B3. Let's double-check with known pattern: For SRA,
  // the encoding is similar to SRL but with funct7=0x20. I'll use the formula:
  // (0x20 << 25) | (rs2 << 20) | (rs1 << 15) | (funct3 << 12) | (rd << 7) |
  // 0x33 rs2=2, rs1=1, funct3=5, rd=3. 0x20 << 25 = 0x40000000 2 << 20 =
  // 0x00200000 1 << 15 = 0x00008000 5 << 12 = 0x00005000 3 << 7  = 0x00000180
  // 0x33    = 0x00000033
  // Sum = 0x40000000 + 0x00200000 = 0x40200000
  // +0x00008000 = 0x40208000
  // +0x00005000 = 0x4020D000
  // +0x00000180 = 0x4020D180
  // +0x33       = 0x4020D1B3
  // So instr = 0x4020D1B3. Let's verify with online or reference: SRA x3, x1,
  // x2 is 0x4020D1B3? Actually I've seen 0x4020D1B3 for SRA? Quick check: For
  // x1=1, x2=2, rd=3, that seems plausible. We'll use that.
  uint32_t instr = 0x4020D1B3;

  CPU cpu;
  cpu.reset();

  // Test 1: arithmetic right shift of positive number (should be same as
  // logical)
  cpu.set_register(CPU::Reg::ra, 0x12345678);
  cpu.set_register(CPU::Reg::sp, 4);
  cpu.write_memory_word(cpu.get_pc(), instr);
  cpu.step();
  TEST(cpu.registers_state()[3] == (0x12345678 >> 4),
       "SRA positive = logical shift");

  // Test 2: arithmetic right shift of negative number (sign extends)
  cpu.reset();
  cpu.set_register(CPU::Reg::ra, 0x80000000); // -2147483648
  cpu.set_register(CPU::Reg::sp, 4);
  cpu.write_memory_word(cpu.get_pc(), instr);
  cpu.step();
  TEST(cpu.registers_state()[3] == 0xF8000000,
       "SRA of 0x80000000 by 4 = 0xF8000000");

  // Test 3: shift by 0 (unchanged)
  cpu.reset();
  cpu.set_register(CPU::Reg::ra, 0xDEADBEEF);
  cpu.set_register(CPU::Reg::sp, 0);
  cpu.write_memory_word(cpu.get_pc(), instr);
  cpu.step();
  TEST(cpu.registers_state()[3] == 0xDEADBEEF, "SRA by 0 unchanged");

  // Test 4: shift by 31 (only sign bit remains)
  cpu.reset();
  cpu.set_register(CPU::Reg::ra, 0x80000000);
  cpu.set_register(CPU::Reg::sp, 31);
  cpu.write_memory_word(cpu.get_pc(), instr);
  cpu.step();
  TEST(cpu.registers_state()[3] == 0xFFFFFFFF,
       "SRA of 0x80000000 by 31 = 0xFFFFFFFF");

  // Test 5: shift amount modulo 32
  cpu.reset();
  cpu.set_register(CPU::Reg::ra, 0x80000000);
  cpu.set_register(CPU::Reg::sp, 33); // 33 mod 32 = 1
  cpu.write_memory_word(cpu.get_pc(), instr);
  cpu.step();
  TEST(cpu.registers_state()[3] == 0xC0000000,
       "SRA by 33 = SRA by 1 (0xC0000000)");

  // Test 6: x0 unchanged
  cpu.reset();
  cpu.set_register(CPU::Reg::ra, 1);
  cpu.set_register(CPU::Reg::sp, 2);
  cpu.write_memory_word(cpu.get_pc(), instr);
  cpu.step();
  TEST(cpu.registers_state()[0] == 0, "x0 remains zero");

  std::cout << "All SRA tests PASSED!" << std::endl;
  return 0;
}
