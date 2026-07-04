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

  // LH x3, 0(x1)   (funct3=1)
  uint32_t instr = make_load(0x1, 1, 3, 0);

  uint32_t data_addr = TEXT_START + 0x100;
  cpu.write_memory_half(data_addr, 0x7FFF);     // +32767
  cpu.write_memory_half(data_addr + 2, 0x8000); // -32768
  cpu.write_memory_half(data_addr + 4, 0xFFFF); // -1

  // Test 1: LH positive halfword
  cpu.set_register(CPU::Reg::ra, data_addr);
  cpu.write_memory_word(cpu.get_pc(), instr);
  cpu.step();
  TEST(cpu.registers_state()[3] == 0x7FFF, "LH 0x7FFF -> 0x7FFF");
  TEST(cpu.get_pc() == TEXT_START + 4, "PC advanced");
  TEST(cpu.registers_state()[0] == 0, "x0 unchanged");

  // Test 2: LH negative halfword (0x8000)
  cpu.reset();
  cpu.set_register(CPU::Reg::ra, data_addr + 2);
  cpu.write_memory_word(cpu.get_pc(), instr);
  cpu.step();
  TEST(cpu.registers_state()[3] == 0xFFFF8000, "LH 0x8000 -> 0xFFFF8000");

  // Test 3: LH negative halfword (0xFFFF)
  cpu.reset();
  cpu.set_register(CPU::Reg::ra, data_addr + 4);
  cpu.write_memory_word(cpu.get_pc(), instr);
  cpu.step();
  TEST(cpu.registers_state()[3] == 0xFFFFFFFF, "LH 0xFFFF -> 0xFFFFFFFF");

  // Test 4: LH with immediate offset (+2)
  cpu.reset();
  cpu.set_register(CPU::Reg::ra, data_addr);
  cpu.write_memory_word(cpu.get_pc(), make_load(0x1, 1, 3, 2));
  cpu.step();
  TEST(cpu.registers_state()[3] == 0xFFFF8000,
       "LH with +2 offset reads 0x8000");

  // Test 5: Unaligned LH (should work because memory is byte‑addressable)
  cpu.reset();
  // Write two bytes at data_addr+1 and data_addr+2
  cpu.write_memory_byte(data_addr + 1, 0x34);
  cpu.write_memory_byte(data_addr + 2, 0x12);
  cpu.set_register(CPU::Reg::ra, data_addr + 1);
  cpu.write_memory_word(cpu.get_pc(), instr);
  cpu.step();
  TEST(cpu.registers_state()[3] ==
           static_cast<uint32_t>(static_cast<int16_t>(0x1234)),
       "Unaligned LH reads 0x1234 (little‑endian)");

  std::cout << "All LH tests PASSED!" << std::endl;
  return 0;
}
