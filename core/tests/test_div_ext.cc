#include "../include/cpu.hpp"
#include "common/instr_encoders.hpp"
#include <cstdint>
#include <iostream>
#include <limits>

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

  const uint32_t A0 = 10, A1 = 11, A2 = 12, A3 = 13, A4 = 14, A5 = 15, A6 = 16,
                 A7 = 17, T0 = 5, T1 = 6, T2 = 7, T3 = 28, T4 = 29, T5 = 30,
                 T6 = 31, S0 = 8, S1 = 9, S2 = 18, S3 = 19, S4 = 20, S5 = 21,
                 S6 = 22, S7 = 23;

  // ----- Test values (same as before) -----
  uint32_t pos_a = 100;
  uint32_t pos_b = 7;
  uint32_t neg_c = static_cast<uint32_t>(-15);
  uint32_t neg_d = static_cast<uint32_t>(-7);
  uint32_t int_min = 0x80000000;
  uint32_t int_max = 0x7FFFFFFF;
  uint32_t all_ones = 0xFFFFFFFF;
  uint32_t zero = 0;

  cpu.set_register(CPU::Reg::a0, pos_a);
  cpu.set_register(CPU::Reg::a1, pos_b);
  cpu.set_register(CPU::Reg::a2, neg_c);
  cpu.set_register(CPU::Reg::a3, neg_d);
  cpu.set_register(CPU::Reg::a4, int_min);
  cpu.set_register(CPU::Reg::a5, int_max);
  cpu.set_register(CPU::Reg::a6, all_ones);
  cpu.set_register(CPU::Reg::a7, zero);

  const uint32_t M_FUNCT7 = 0x01;

  uint32_t code[] = {// 1: div   t0, a0, a1
                     make_r_type(M_FUNCT7, A1, A0, 0b100, T0),
                     // 2: rem   t1, a0, a1
                     make_r_type(M_FUNCT7, A1, A0, 0b110, T1),
                     // 3: divu  t2, a0, a1
                     make_r_type(M_FUNCT7, A1, A0, 0b101, T2),
                     // 4: remu  t3, a0, a1
                     make_r_type(M_FUNCT7, A1, A0, 0b111, T3),
                     // 5: div   t4, a2, a1
                     make_r_type(M_FUNCT7, A1, A2, 0b100, T4),
                     // 6: rem   t5, a2, a1
                     make_r_type(M_FUNCT7, A1, A2, 0b110, T5),
                     // 7: div   t6, a2, a3
                     make_r_type(M_FUNCT7, A3, A2, 0b100, T6),
                     // 8: rem   s0, a2, a3
                     make_r_type(M_FUNCT7, A3, A2, 0b110, S0),
                     // 9: div   s1, a0, a3
                     make_r_type(M_FUNCT7, A3, A0, 0b100, S1),
                     // 10: rem   s2, a0, a3
                     make_r_type(M_FUNCT7, A3, A0, 0b110, S2),
                     // 11: div   s3, a4, a6
                     make_r_type(M_FUNCT7, A6, A4, 0b100, S3),
                     // 12: rem   s4, a4, a6
                     make_r_type(M_FUNCT7, A6, A4, 0b110, S4),
                     // 13: div   s5, a4, a1
                     make_r_type(M_FUNCT7, A1, A4, 0b100, S5),
                     // 14: rem   s6, a4, a1
                     make_r_type(M_FUNCT7, A1, A4, 0b110, S6),
                     // 15: div   s7, a6, a1
                     make_r_type(M_FUNCT7, A1, A6, 0b100, S7),
                     // 16: rem   t0, a6, a1   (overwrites t0)
                     make_r_type(M_FUNCT7, A1, A6, 0b110, T0),
                     // 17: div   t1, a0, a7   (overwrites t1)
                     make_r_type(M_FUNCT7, A7, A0, 0b100, T1),
                     // 18: rem   t2, a0, a7   (overwrites t2)
                     make_r_type(M_FUNCT7, A7, A0, 0b110, T2),
                     // 19: divu  t3, a0, a7   (overwrites t3)
                     make_r_type(M_FUNCT7, A7, A0, 0b101, T3),
                     // 20: remu  t4, a0, a7   (overwrites t4)
                     make_r_type(M_FUNCT7, A7, A0, 0b111, T4)};
  const int num_instr = sizeof(code) / sizeof(code[0]);

  for (int i = 0; i < num_instr; ++i)
    cpu.write_memory_word(TEXT_START + i * 4, code[i]);

  // ----- Expected values (same as before) -----
  int32_t s_pos_a = static_cast<int32_t>(pos_a);
  int32_t s_pos_b = static_cast<int32_t>(pos_b);
  int32_t s_neg_c = static_cast<int32_t>(neg_c);
  int32_t s_neg_d = static_cast<int32_t>(neg_d);
  int32_t s_int_min = static_cast<int32_t>(int_min);
  int32_t s_all_ones = static_cast<int32_t>(all_ones);

  uint32_t exp[21] = {0};                             // index 1..20
  exp[1] = static_cast<uint32_t>(s_pos_a / s_pos_b);  // 14
  exp[2] = static_cast<uint32_t>(s_pos_a % s_pos_b);  // 2
  exp[3] = pos_a / pos_b;                             // 14
  exp[4] = pos_a % pos_b;                             // 2
  exp[5] = static_cast<uint32_t>(s_neg_c / s_pos_b);  // -2
  exp[6] = static_cast<uint32_t>(s_neg_c % s_pos_b);  // -1
  exp[7] = static_cast<uint32_t>(s_neg_c / s_neg_d);  // 2
  exp[8] = static_cast<uint32_t>(s_neg_c % s_neg_d);  // -1
  exp[9] = static_cast<uint32_t>(s_pos_a / s_neg_d);  // -14
  exp[10] = static_cast<uint32_t>(s_pos_a % s_neg_d); // 2
  exp[11] = int_min;                                  // overflow
  exp[12] = 0;
  exp[13] = static_cast<uint32_t>(s_int_min / 7);  // -306783378
  exp[14] = static_cast<uint32_t>(s_int_min % 7);  // -2
  exp[15] = static_cast<uint32_t>(s_all_ones / 7); // 0
  exp[16] = static_cast<uint32_t>(s_all_ones % 7); // -1
  exp[17] = 0xFFFFFFFF;                            // div by zero
  exp[18] = pos_a;                                 // rem by zero
  exp[19] = 0xFFFFFFFF;                            // divu by zero
  exp[20] = pos_a;                                 // remu by zero

  // ----- Step‑by‑step verification -----
  uint32_t pc = TEXT_START;
  auto regs = cpu.registers_state();

// Helper macro: step, then check a register against expected value
#define STEP_CHECK(reg, expected, msg)                                         \
  do {                                                                         \
    TEST(cpu.step(), "step " msg);                                             \
    TEST(cpu.get_pc() == pc + 4, "PC increment " msg);                         \
    pc += 4;                                                                   \
    TEST(cpu.registers_state()[reg] == (expected), msg);                       \
  } while (0)

  STEP_CHECK(T0, exp[1], "div 100,7");
  STEP_CHECK(T1, exp[2], "rem 100,7");
  STEP_CHECK(T2, exp[3], "divu 100,7");
  STEP_CHECK(T3, exp[4], "remu 100,7");
  STEP_CHECK(T4, exp[5], "div -15,7");
  STEP_CHECK(T5, exp[6], "rem -15,7");
  STEP_CHECK(T6, exp[7], "div -15,-7");
  STEP_CHECK(S0, exp[8], "rem -15,-7");
  STEP_CHECK(S1, exp[9], "div 100,-7");
  STEP_CHECK(S2, exp[10], "rem 100,-7");
  STEP_CHECK(S3, exp[11], "div INT_MIN,-1");
  STEP_CHECK(S4, exp[12], "rem INT_MIN,-1");
  STEP_CHECK(S5, exp[13], "div INT_MIN,7");
  STEP_CHECK(S6, exp[14], "rem INT_MIN,7");
  STEP_CHECK(S7, exp[15], "div -1,7");
  STEP_CHECK(T0, exp[16], "rem -1,7");   // T0 overwritten
  STEP_CHECK(T1, exp[17], "div 100,0");  // T1 overwritten
  STEP_CHECK(T2, exp[18], "rem 100,0");  // T2 overwritten
  STEP_CHECK(T3, exp[19], "divu 100,0"); // T3 overwritten
  STEP_CHECK(T4, exp[20], "remu 100,0"); // T4 overwritten

  TEST(cpu.registers_state()[0] == 0, "x0 still zero");

  std::cout << "All division/remainder tests PASSED!" << std::endl;
  return 0;
}
