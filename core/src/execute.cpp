#include "../include/execute.hpp"
#include "../include/decode.hpp"
#include "../include/memory.hpp"
#include "../include/register.hpp"

namespace riscv {
namespace execute {

uint32_t execute_i_type(const DecodedInstruction &d_instr, uint32_t current_pc,
                        RegisterFile & /*regs*/, Memory & /*mem*/) {
  // Original stub: just return current_pc
  return current_pc;
}

uint32_t execute_b_type(const DecodedInstruction &d_instr, uint32_t current_pc,
                        RegisterFile & /*regs*/, Memory & /*mem*/) {
  // Original stub: just return current_pc
  return current_pc;
}

} // namespace execute
} // namespace riscv
