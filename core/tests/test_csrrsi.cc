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
uint32_t make_csri(uint32_t csr_addr, uint32_t zimm, uint32_t funct3,
                   uint32_t rd) {
  return (csr_addr << 20) | (zimm << 15) | (funct3 << 12) | (rd << 7) |
         0b1110011;
}
} // namespace rv32i

int main() {
  using namespace rv32i;

  CPU cpu;
  cpu.reset();

  constexpr uint16_t kCsr = 0x340;

  cpu.write_csr(kCsr, 0x0F0F);

  // CSRRSI x11, 0x340, 0x10  (funct3 = 110)
  cpu.write_memory_word(cpu.get_pc(),
                        make_csri(kCsr, /*zimm=*/0x10, /*funct3=*/0b110,
                                  /*rd=*/11));
  bool ok = cpu.step();
  TEST(ok, "CSRRSI step succeeds");
  TEST(cpu.registers_state()[11] == 0x0F0F, "CSRRSI returns old CSR value");
  TEST(cpu.read_csr(kCsr) == (0x0F0F | 0x10), "CSRRSI ORs zimm into CSR");

  std::cout << "PASS: test_csrrsi" << std::endl;
  return 0;
}
