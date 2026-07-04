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

  CPU cpu;
  cpu.reset();

  // LB x3, 0(x1)   (funct3=0)
  uint32_t instr = make_load(0x0, 1, 3, 0);

  // Place test data at TEXT_START + 0x100
  uint32_t data_addr = TEXT_START + 0x100;
  cpu.write_memory_byte(data_addr, 0x80);     // -128
  cpu.write_memory_byte(data_addr + 1, 0x7F); // +127
  cpu.write_memory_byte(data_addr + 2, 0xFF); // -1
  cpu.write_memory_byte(data_addr + 3, 0x00); // 0

  // Test 1: LB positive byte (0x7F)
  cpu.set_register(CPU::Reg::ra, data_addr + 1); // x1 = data_addr+1
  cpu.write_memory_word(cpu.get_pc(), instr);
  cpu.step();
  TEST(cpu.registers_state()[3] == 0x7F, "LB 0x7F -> 0x7F");
  TEST(cpu.get_pc() == TEXT_START + 4, "PC advanced");
  TEST(cpu.registers_state()[0] == 0, "x0 unchanged");

  // Test 2: LB negative byte (0x80)
  cpu.reset();
  cpu.set_register(CPU::Reg::ra, data_addr);
  cpu.write_memory_word(cpu.get_pc(), instr);
  cpu.step();
  TEST(cpu.registers_state()[3] == 0xFFFFFF80, "LB 0x80 -> 0xFFFFFF80");

  // Test 3: LB negative byte (0xFF)
  cpu.reset();
  cpu.set_register(CPU::Reg::ra, data_addr + 2);
  cpu.write_memory_word(cpu.get_pc(), instr);
  cpu.step();
  TEST(cpu.registers_state()[3] == 0xFFFFFFFF, "LB 0xFF -> 0xFFFFFFFF");

  // Test 4: LB with immediate offset (+2)
  cpu.reset();
  cpu.set_register(CPU::Reg::ra, data_addr);
  cpu.write_memory_word(cpu.get_pc(), make_load(0x0, 1, 3, 2)); // offset +2
  cpu.step();
  TEST(cpu.registers_state()[3] == 0xFFFFFFFF, "LB with +2 offset reads 0xFF");

  // Test 5: LB from address that causes exception (optional – depends on memory
  // bounds) If your memory throws on out-of-bounds, you might test that here.
  // For simplicity, we skip.

  std::cout << "All LB tests PASSED!" << std::endl;
  return 0;
}
