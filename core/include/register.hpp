#ifndef RISCV_REGISTER_HPP
#define RISCV_REGISTER_HPP

#include <array>
#include <cassert>
#include <cstdint>

namespace riscv {

class RegisterFile {
public:
  RegisterFile() = default;

  void reset() {
    for (auto &reg : regs)
      reg = 0;
  }

  uint32_t read(uint32_t reg_index) const { return regs[reg_index]; }

  void write(uint32_t reg_index, uint32_t value) {
    // protection of invalid indices added as well.
    assert(reg_index < 32);
    // x0 is hardwired to zero; writing is allowed but will be ignored by the
    // ISA.
    if (reg_index == 0)
      return;

    regs[reg_index] = value;
  }

  const std::array<uint32_t, 32> &get_array() const { return regs; }

private:
  std::array<uint32_t, 32> regs{};
};

} // namespace riscv

#endif
