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

  // ---- Test values ----
  uint32_t val_a = 0x7FFFFFFF; //  2^31-1 (max signed positive)
  uint32_t val_b = 0x7FFFFFFF;
  uint32_t val_c = 0xFFFFFFFF; // -1 (signed) / 2^32-1 (unsigned)
  uint32_t val_d = 0xFFFFFFFF;
  uint32_t val_x = 100;
  uint32_t val_y = 7;
  uint32_t val_zero = 0;
  uint32_t val_neg = static_cast<uint32_t>(-15);
  uint32_t val_min = 0x80000000; // -2^31

  // Register indices
  const uint32_t A0 = 10, A1 = 11, A2 = 12, A3 = 13, A4 = 14, A5 = 15, A6 = 16,
                 A7 = 17, T0 = 5;

  cpu.set_register(CPU::Reg::a0, val_a);
  cpu.set_register(CPU::Reg::a1, val_b);
  cpu.set_register(CPU::Reg::a2, val_c);
  cpu.set_register(CPU::Reg::a3, val_d);
  cpu.set_register(CPU::Reg::a4, val_x);
  cpu.set_register(CPU::Reg::a5, val_y);
  cpu.set_register(CPU::Reg::a6, val_zero);
  cpu.set_register(CPU::Reg::a7, val_neg);
  cpu.set_register(CPU::Reg::t0, val_min);

  // ---- Build instruction sequence ----
  const uint32_t M_FUNCT7 = 0x01; // M extension

  // Destination registers: t1=6, t2=7, t3=28, t4=29, t5=30, t6=31,
  // s0=8, s1=9, s2=18, s3=19, s4=20, s5=21, s6=22, s7=23
  uint32_t code[] = {// mul   t1, a0, a1
                     make_r_type(M_FUNCT7, A1, A0, 0b000, 6),
                     // mulh  t2, a0, a1
                     make_r_type(M_FUNCT7, A1, A0, 0b001, 7),
                     // mulhu t3, a2, a3
                     make_r_type(M_FUNCT7, A3, A2, 0b011, 28),
                     // mulhsu t4, a0, a3
                     make_r_type(M_FUNCT7, A3, A0, 0b010, 29),
                     // div   t5, a4, a5
                     make_r_type(M_FUNCT7, A5, A4, 0b100, 30),
                     // divu  t6, a4, a5
                     make_r_type(M_FUNCT7, A5, A4, 0b101, 31),
                     // rem   s0, a4, a5
                     make_r_type(M_FUNCT7, A5, A4, 0b110, 8),
                     // remu  s1, a4, a5
                     make_r_type(M_FUNCT7, A5, A4, 0b111, 9),
                     // div   s2, a4, a6   (division by zero)
                     make_r_type(M_FUNCT7, A6, A4, 0b100, 18),
                     // rem   s3, a4, a6   (remainder by zero)
                     make_r_type(M_FUNCT7, A6, A4, 0b110, 19),
                     // div   s4, a7, a5   (-15 / 7)
                     make_r_type(M_FUNCT7, A5, A7, 0b100, 20),
                     // rem   s5, a7, a5   (-15 % 7)
                     make_r_type(M_FUNCT7, A5, A7, 0b110, 21),
                     // div   s6, t0, a7   (-2^31 / -15)
                     make_r_type(M_FUNCT7, A7, T0, 0b100, 22),
                     // rem   s7, t0, a7   (-2^31 % -15)
                     make_r_type(M_FUNCT7, A7, T0, 0b110, 23)};
  const int num_instr = sizeof(code) / sizeof(code[0]);

  for (int i = 0; i < num_instr; ++i)
    cpu.write_memory_word(TEXT_START + i * 4, code[i]);

  // ---- Expected results ----
  uint64_t prod = static_cast<uint64_t>(val_a) * val_b;
  uint32_t exp_t1 = static_cast<uint32_t>(prod);       // 1
  uint32_t exp_t2 = static_cast<uint32_t>(prod >> 32); // 0x3FFFFFFF

  uint64_t prod_uu = static_cast<uint64_t>(val_c) * val_d;
  uint32_t exp_t3 = static_cast<uint32_t>(prod_uu >> 32); // 0xFFFFFFFE

  int64_t prod_su = static_cast<int64_t>(static_cast<int32_t>(val_a)) *
                    static_cast<int64_t>(static_cast<uint32_t>(val_d));
  uint32_t exp_t4 =
      static_cast<uint32_t>(static_cast<uint64_t>(prod_su) >> 32); // 0x7FFFFFFE

  uint32_t exp_t5 = 100 / 7; // 14
  uint32_t exp_t6 = 100 / 7; // 14
  uint32_t exp_s0 = 100 % 7; // 2
  uint32_t exp_s1 = 100 % 7; // 2

  uint32_t exp_s2 = 0xFFFFFFFF; // div by zero
  uint32_t exp_s3 = 100;        // rem by zero

  uint32_t exp_s4 =
      static_cast<uint32_t>(static_cast<int32_t>(-15) / 7); // 0xFFFFFFFE
  uint32_t exp_s5 =
      static_cast<uint32_t>(static_cast<int32_t>(-15) % 7); // 0xFFFFFFFF

  int32_t min = static_cast<int32_t>(0x80000000);
  int32_t neg15 = -15;
  uint32_t exp_s6 =
      static_cast<uint32_t>(min / neg15); // 143165576 (0x08888888)
  uint32_t exp_s7 = static_cast<uint32_t>(min % neg15); // 0xFFFFFFF8

  // ---- Execute and verify ----
  uint32_t pc = TEXT_START;
  for (int i = 0; i < num_instr; ++i) {
    TEST(cpu.step(), "step execution");
    TEST(cpu.get_pc() == pc + 4, "PC increment");
    pc += 4;
  }

  TEST(cpu.registers_state()[6] == exp_t1, "t1");
  TEST(cpu.registers_state()[7] == exp_t2, "t2");
  TEST(cpu.registers_state()[28] == exp_t3, "t3");
  TEST(cpu.registers_state()[29] == exp_t4, "t4");
  TEST(cpu.registers_state()[30] == exp_t5, "t5");
  TEST(cpu.registers_state()[31] == exp_t6, "t6");
  TEST(cpu.registers_state()[8] == exp_s0, "s0");
  TEST(cpu.registers_state()[9] == exp_s1, "s1");
  TEST(cpu.registers_state()[18] == exp_s2, "s2");
  TEST(cpu.registers_state()[19] == exp_s3, "s3");
  TEST(cpu.registers_state()[20] == exp_s4, "s4");
  TEST(cpu.registers_state()[21] == exp_s5, "s5");
  TEST(cpu.registers_state()[22] == exp_s6, "s6");
  TEST(cpu.registers_state()[23] == exp_s7, "s7");

  TEST(cpu.registers_state()[0] == 0, "x0 still zero");

  std::cout << "All M‑extension tests PASSED!" << std::endl;
  return 0;
}
