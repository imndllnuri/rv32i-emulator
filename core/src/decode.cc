#include "../include/decode.hpp"
#include "../include/exception.hpp"
#include "../include/instruction.hpp"

namespace rv32i {

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
  case opcode::R_TYPE:
    d.format = DecodedInstruction::Format::R;
    d.imm = 0;
    break;
  case opcode::I_TYPE:
    d.format = DecodedInstruction::Format::I;
    d.imm = imm_i(instr);
    break;
  case opcode::I_TYPE_L:
    d.format = DecodedInstruction::Format::I;
    d.imm = static_cast<int32_t>(instr) >> 20;
    break;
  case opcode::I_TYPE_S:
    d.format = DecodedInstruction::Format::S;
    d.imm = imm_s(instr);
    break;
  case opcode::B_TYPE:
    d.format = DecodedInstruction::Format::B;
    d.imm = imm_b(instr);
    break;
  case opcode::JAL:
    d.format = DecodedInstruction::Format::J;
    d.imm = imm_j(instr);
    break;
  case opcode::JALR:
    d.format = DecodedInstruction::Format::I; // I‑tipi
    d.imm = imm_i(instr);                     // 12-bit signed immediate
    break;
  case opcode::LUI:
  case opcode::AUIPC:
    d.format = DecodedInstruction::Format::U;
    d.imm = imm_u(instr);
    break;
  case opcode::SYSTEM:
    d.format = DecodedInstruction::Format::I;
    d.imm = imm_i(instr); // 12‑bit signed immediate
    break;
  default:
    throw IllegalInstructionException("Instruction is not implemented yet.");
  }

  return d;
}

} // namespace rv32i
