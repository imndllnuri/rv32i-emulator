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
// FENCE/FENCE.I share the I-type layout; pred/succ/fm bits in the immediate
// are irrelevant since this core has no caches or multiple harts.
uint32_t make_fence(uint32_t funct3) {
  return (funct3 << 12) | 0b0001111; // rd=0, rs1=0, imm=0
}
} // namespace rv32i

int main() {
  using namespace rv32i;

  CPU cpu;
  cpu.reset();

  // FENCE (funct3 = 0)
  cpu.set_register(CPU::Reg::a0, 0x1234);
  cpu.write_memory_word(cpu.get_pc(), make_fence(0b000));
  bool ok = cpu.step();
  TEST(ok, "FENCE step succeeds");
  TEST(cpu.get_pc() == TEXT_START + 4, "FENCE advances PC by 4");
  TEST(cpu.registers_state()[static_cast<uint32_t>(CPU::Reg::a0)] == 0x1234,
       "FENCE leaves registers untouched");

  // FENCE.I (funct3 = 1)
  cpu.reset();
  cpu.write_memory_word(cpu.get_pc(), make_fence(0b001));
  ok = cpu.step();
  TEST(ok, "FENCE.I step succeeds");
  TEST(cpu.get_pc() == TEXT_START + 4, "FENCE.I advances PC by 4");

  std::cout << "PASS: test_fence" << std::endl;
  return 0;
}
