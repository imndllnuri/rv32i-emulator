#include "../include/decode.hpp"
#include "../include/instruction.hpp"

namespace riscv {

static int32_t imm_i(uint32_t instr) {
  return static_cast<int32_t>(instr) >> 20;
}

static int32_t imm_s(uint32_t instr) {
  int32_t imm = ((instr >> 25) & 0x7F) << 5 | ((instr >> 7) & 0x1F);
  if (imm & 0x800)
    imm |= 0xFFFFF000;
  return imm;
}

static int32_t imm_b(uint32_t instr) {
  int32_t imm = ((instr >> 31) & 0x1) << 12 | ((instr >> 7) & 0x1) << 11 |
                ((instr >> 25) & 0x3F) << 5 | ((instr >> 8) & 0xF) << 1;
  if (imm & 0x1000)
    imm |= 0xFFFFE000;
  return imm;
}

static int32_t imm_u(uint32_t instr) {
  return static_cast<int32_t>(instr & 0xFFFFF000);
}

static int32_t imm_j(uint32_t instr) {
  int32_t imm = ((instr >> 31) & 0x1) << 20 | ((instr >> 12) & 0xFF) << 12 |
                ((instr >> 20) & 0x1) << 11 | ((instr >> 21) & 0x3FF) << 1;
  if (imm & 0x100000)
    imm |= 0xFFE00000;
  return imm;
}

DecodedInstruction decode(uint32_t instr) {
  DecodedInstruction d;
  d.opcode = instr & 0x7F;
  d.rd = (instr >> 7) & 0x1F;
  d.funct3 = (instr >> 12) & 0x7;
  d.rs1 = (instr >> 15) & 0x1F;
  d.rs2 = (instr >> 20) & 0x1F;
  d.funct7 = (instr >> 25) & 0x7F;

  // Determine format and compute immediate
  switch (d.opcode) {
  case opcode::IRRO:
    d.format = DecodedInstruction::Format::R;
    d.imm = 0;
    break;
  default:
    d.format = DecodedInstruction::Format::R;
    d.imm = 0;
    break;
  }

  return d;
}

} // namespace riscv
