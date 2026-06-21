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
uint32_t make_csr(uint32_t csr_addr, uint32_t rs1, uint32_t funct3,
                  uint32_t rd) {
  return (csr_addr << 20) | (rs1 << 15) | (funct3 << 12) | (rd << 7) |
         0b1110011;
}
} // namespace rv32i

int main() {
  using namespace rv32i;

  CPU cpu;
  cpu.reset();

  constexpr uint16_t kCsr = 0x340;

  cpu.write_csr(kCsr, 0x0FFF);
  cpu.set_register(CPU::Reg::a0, 0x00F0); // rs1 = x10 (clear mask)

  // CSRRC x11, 0x340, x10  (funct3 = 011)
  cpu.write_memory_word(cpu.get_pc(),
                        make_csr(kCsr, /*rs1=*/10, /*funct3=*/0b011,
                                 /*rd=*/11));
  bool ok = cpu.step();
  TEST(ok, "CSRRC step succeeds");
  TEST(cpu.registers_state()[11] == 0x0FFF, "CSRRC returns old CSR value");
  TEST(cpu.read_csr(kCsr) == (0x0FFF & ~0x00F0), "CSRRC clears rs1 bits in CSR");

  std::cout << "PASS: test_csrrc" << std::endl;
  return 0;
}
