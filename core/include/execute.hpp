#ifndef RISCV_EXECUTE_HPP
#define RISCV_EXECUTE_HPP

#include "decode.hpp"
#include <cstdint>

namespace rv32i {

class RegisterFile;
class Memory;

namespace execute {

uint32_t execute_r_type(const DecodedInstruction &d_instr, uint32_t current_pc,
                        RegisterFile &regs, Memory &mem);
uint32_t execute_i_type(const DecodedInstruction &d_instr, uint32_t current_pc,
                        RegisterFile &regs, Memory &mem);
uint32_t execute_s_type(const DecodedInstruction &d_instr, uint32_t current_pc,
                        RegisterFile &regs, Memory &mem);
uint32_t execute_b_type(const DecodedInstruction &d_instr, uint32_t current_pc,
                        RegisterFile &regs, Memory &mem);
uint32_t execute_u_type(const DecodedInstruction &d_instr, uint32_t current_pc,
                        RegisterFile &regs, Memory &mem);
uint32_t execute_j_type(const DecodedInstruction &d_instr, uint32_t current_pc,
                        RegisterFile &regs, Memory &mem);

} // namespace execute
} // namespace rv32i

#endif
