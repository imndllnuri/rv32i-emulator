// Register ABI Name Description Saver
// x0 zero Zero constant —
// x1 ra Return address Callee
// x2 sp Stack pointer Callee
// x3 gp Global pointer —
// x4 tp Thread pointer —
// x5-x7 t0-t2 Temporaries Caller
// x8 s0 / fp Saved / frame pointer Callee
// x9 s1 Saved register Callee
// x10-x11 a0-a1 Fn args/return values Caller
// x12-x17 a2-a7 Fn args Caller
// x18-x27 s2-s11 Saved registers Callee
// x28-x31 t3-t6 Temporaries Caller
// f0-7 ft0-7 FP temporaries Caller
// f8-9 fs0-1 FP saved registers Callee
// f10-11 fa0-1 FP args/return values Caller
// f12-17 fa2-7 FP args Caller
// f18-27 fs2-11 FP saved registers Callee
// f28-31 ft8-11 FP temporaries Caller
//
#ifndef RISCV_REGISTER_HPP
#define RISCV_REGISTER_HPP

#include <array>
#include <cassert>
#include <cstdint>

namespace rv32i {

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

} // namespace rv32i

#endif
