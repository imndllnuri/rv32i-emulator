#include "../include/cpu.hpp"
#include "common/instr_encoders.hpp"
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

  // SLTI x3, x1, 5   (funct3=2)
  uint32_t instr = make_i_type(0x2, 1, 3, 5);

  CPU cpu;
  cpu.reset();

  // Test 1: positive less than positive
  cpu.set_register(CPU::Reg::ra, 3);
  cpu.write_memory_word(cpu.get_pc(), instr);
  cpu.step();
  // std::cout << cpu.registers_state()[3] << std::endl;
  TEST(cpu.registers_state()[3] == 1, "3 < 5 -> 1");

  // Test 2: positive not less than positive
  cpu.reset();
  cpu.set_register(CPU::Reg::ra, 7);
  cpu.write_memory_word(cpu.get_pc(), instr);
  cpu.step();
  TEST(cpu.registers_state()[3] == 0, "7 < 5 -> 0");

  // Test 3: negative less than positive
  cpu.reset();
  cpu.set_register(CPU::Reg::ra, (uint32_t)-5); // 0xFFFFFFFB
  cpu.write_memory_word(cpu.get_pc(), instr);
  cpu.step();
  TEST(cpu.registers_state()[3] == 1, "-5 < 5 -> 1");

  // Test 4: positive less than negative (false)
  cpu.reset();
  cpu.set_register(CPU::Reg::ra, 10);
  cpu.write_memory_word(cpu.get_pc(), make_i_type(0x2, 1, 3, -5)); // imm = -5
  cpu.step();
  TEST(cpu.registers_state()[3] == 0, "10 < -5 -> 0");

  // Test 5: negative less than negative
  cpu.reset();
  cpu.set_register(CPU::Reg::ra, (uint32_t)-10);
  cpu.write_memory_word(cpu.get_pc(), make_i_type(0x2, 1, 3, -5));
  cpu.step();
  TEST(cpu.registers_state()[3] == 1, "-10 < -5 -> 1");

  // Test 6: equal values
  cpu.reset();
  cpu.set_register(CPU::Reg::ra, 42);
  cpu.write_memory_word(cpu.get_pc(), make_i_type(0x2, 1, 3, 42));
  cpu.step();
  TEST(cpu.registers_state()[3] == 0, "42 < 42 -> 0");

  // Test 7: x0 unchanged
  cpu.reset();
  cpu.set_register(CPU::Reg::ra, 1);
  cpu.write_memory_word(cpu.get_pc(), instr);
  cpu.step();
  TEST(cpu.registers_state()[0] == 0, "x0 remains zero");

  std::cout << "All SLTI tests PASSED!" << std::endl;
  return 0;
}
