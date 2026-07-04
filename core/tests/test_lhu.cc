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

  // LHU x3, 0(x1)   (funct3=5)
  uint32_t instr = make_load(0x5, 1, 3, 0);

  uint32_t data_addr = TEXT_START + 0x100;
  cpu.write_memory_half(data_addr, 0x8000);     // 32768
  cpu.write_memory_half(data_addr + 2, 0xFFFF); // 65535
  cpu.write_memory_half(data_addr + 4, 0x1234); // 4660

  // Test 1: LHU of 0x8000 -> 0x8000
  cpu.set_register(CPU::Reg::ra, data_addr);
  cpu.write_memory_word(cpu.get_pc(), instr);
  cpu.step();
  TEST(cpu.registers_state()[3] == 0x8000, "LHU 0x8000 -> 0x8000");
  TEST(cpu.get_pc() == TEXT_START + 4, "PC advanced");
  TEST(cpu.registers_state()[0] == 0, "x0 unchanged");

  // Test 2: LHU of 0xFFFF -> 0xFFFF (not sign‑extended)
  cpu.reset();
  cpu.set_register(CPU::Reg::ra, data_addr + 2);
  cpu.write_memory_word(cpu.get_pc(), instr);
  cpu.step();
  TEST(cpu.registers_state()[3] == 0xFFFF, "LHU 0xFFFF -> 0xFFFF");

  // Test 3: LHU with immediate offset (+2)
  cpu.reset();
  cpu.set_register(CPU::Reg::ra, data_addr);
  cpu.write_memory_word(cpu.get_pc(), make_load(0x5, 1, 3, 2));
  cpu.step();
  TEST(cpu.registers_state()[3] == 0xFFFF, "LHU with +2 offset reads 0xFFFF");

  // Test 4: Unaligned LHU
  cpu.reset();
  cpu.write_memory_byte(data_addr + 1, 0x34);
  cpu.write_memory_byte(data_addr + 2, 0x12);
  cpu.set_register(CPU::Reg::ra, data_addr + 1);
  cpu.write_memory_word(cpu.get_pc(), instr);
  cpu.step();
  TEST(cpu.registers_state()[3] == 0x1234,
       "Unaligned LHU reads 0x1234 (little‑endian)");

  std::cout << "All LHU tests PASSED!" << std::endl;
  return 0;
}
