#ifndef RISCV_EXECUTE_HPP
#define RISCV_EXECUTE_HPP

#include "decode.hpp"
#include <cstdint>

namespace riscv {

class RegisterFile;
class Memory;

namespace execute {

// Execution stubs – preserve the original behaviour (return current_pc)
uint32_t execute_i_type(const DecodedInstruction &d_instr, uint32_t current_pc,
                        RegisterFile &regs, Memory &mem);
uint32_t execute_b_type(const DecodedInstruction &d_instr, uint32_t current_pc,
                        RegisterFile &regs, Memory &mem);

} // namespace execute
} // namespace riscv

#endif
