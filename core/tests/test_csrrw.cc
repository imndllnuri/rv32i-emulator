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

  constexpr uint16_t kCsr = 0x340; // arbitrary CSR address

  cpu.write_csr(kCsr, 0xAAAA);
  cpu.set_register(CPU::Reg::a0, 0x1234); // rs1 = x10

  // CSRRW x11, 0x340, x10  (funct3 = 001)
  cpu.write_memory_word(cpu.get_pc(),
                        make_csr(kCsr, /*rs1=*/10, /*funct3=*/0b001,
                                 /*rd=*/11));
  bool ok = cpu.step();
  TEST(ok, "CSRRW step succeeds");
  TEST(cpu.registers_state()[11] == 0xAAAA, "CSRRW returns old CSR value");
  TEST(cpu.read_csr(kCsr) == 0x1234, "CSRRW writes rs1 into CSR");
  TEST(cpu.get_pc() == TEXT_START + 4, "CSRRW advances PC by 4");

  std::cout << "PASS: test_csrrw" << std::endl;
  return 0;
}
