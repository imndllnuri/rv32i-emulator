#include "../include/execute.hpp"
#include "../include/decode.hpp"
#include "../include/exception.hpp" // your custom exceptions
#include "../include/instruction.hpp"
#include "../include/memory.hpp"
#include "../include/register.hpp"
#include <cstdint>

namespace riscv {
namespace execute {

uint32_t execute_r_type(const DecodedInstruction &d_instr, uint32_t current_pc,
                        RegisterFile &regs, Memory & /*mem*/) {
  if (d_instr.opcode != opcode::R_TYPE) {
    throw IllegalInstructionException("Non‑R‑type opcode in R‑type handler");
  }

  uint32_t rs1 = regs.read(d_instr.rs1);
  uint32_t rs2 = regs.read(d_instr.rs2);
  uint32_t rd;

  switch (d_instr.funct3) {
  case funct3::ADD_SUB:
    if (d_instr.funct7 == funct7::BASE) {
      rd = rs1 + rs2; // ADD
    } else if (d_instr.funct7 == funct7::SUB_SRA) {
      rd = rs1 - rs2; // SUB
    } else {
      throw IllegalInstructionException("R‑type ADD/SUB with wrong funct7");
    }
    break;

  case funct3::XOR:                       // XOR = 4
    if (d_instr.funct7 == funct7::BASE) { // BASE = 0
      rd = rs1 ^ rs2;
    } else {
      throw IllegalInstructionException("R‑type XOR with wrong funct7");
    }
    break;

  case funct3::OR: // or = 6 0b
    if (d_instr.funct7 == funct7::BASE) {
      rd = rs1 | rs2;
    } else {
      throw IllegalInstructionException("R‑type OR with wrong funct7");
    }
    break;

  case funct3::AND:
    if (d_instr.funct7 == funct7::BASE) {
      rd = rs1 & rs2;
    } else {
      throw IllegalInstructionException("R‑type AND with wrong funct7");
    }
    break;

  case funct3::SLL:
    if (d_instr.funct7 == funct7::BASE) {
      rd = rs1 << (rs2 & 0x1F);
    } else {
      throw IllegalInstructionException("R‑type SLL with wrong funct7");
    }
    break;

  case funct3::SRL_SRA:
    if (d_instr.funct7 == funct7::SUB_SRA) {
      rd = static_cast<int32_t>(rs1) >> (rs2 & 0x1F); // SRA
    } else if (d_instr.funct7 == funct7::BASE) {
      rd = rs1 >> (rs2 & 0x1F); // SRL
    } else {
      throw IllegalInstructionException("R‑type SRL/SRA with wrong funct7");
    }
    break;

  case funct3::SLT:
    if (d_instr.funct7 == funct7::BASE) {
      rd = (static_cast<int32_t>(rs1) < static_cast<int32_t>(rs2)) ? 1u : 0u;
    } else {
      throw IllegalInstructionException("R‑type SLT with wrong funct7");
    }
    break;

  case funct3::SLTU:
    if (d_instr.funct7 == funct7::BASE) {
      rd = (rs1 < rs2) ? 1u : 0u;
    } else {
      throw IllegalInstructionException("R‑type SLTU with wrong funct7");
    }
    break;

  default:
    throw UnimplementedInstructionException("R‑type funct3 not implemented");
  }

  regs.write(d_instr.rd, rd);
  return current_pc + 4;
}

// For other types, either implement or throw
uint32_t execute_i_type(const DecodedInstruction &d_instr, uint32_t current_pc,
                        RegisterFile &regs, Memory & /*mem*/) {
  if (d_instr.opcode != opcode::I_TYPE) {
    throw IllegalInstructionException("Non I-Type opcode in I-Type handler.");
  }
  uint32_t rs1 = regs.read(d_instr.rs1);
  uint32_t imm = d_instr.imm;
  uint32_t rd;
  switch (d_instr.funct3) {

  case funct3::ADDI:
    rd = rs1 + imm;
    break;
  case funct3::XORI:
    rd = rs1 ^ imm;
    break;
  case funct3::ORI:
    rd = rs1 | imm;
    break;
  case funct3::ANDI:
    rd = rs1 & imm;
    break;
  case funct3::SLLI:
    rd = rs1 << (imm & 0x1F);
    break;
  case funct3::SRLI_SRAI:
    if (d_instr.funct7 == funct7::BASE) {
      rd = rs1 >> (imm & 0x1F);
      break;
    } else if (d_instr.funct7 == funct7::SUB_SRA) {
      rd = static_cast<int32_t>(rs1) >> (imm & 0x1F);
      break;
    } else {
      throw IllegalInstructionException("I-Type SRLI/SRAI with wrong funct7.");
    }
  case funct3::SLTI:
    rd = (static_cast<int32_t>(rs1) < static_cast<int32_t>(imm)) ? 1 : 0;
    break;
  case funct3::SLTIU:
    rd = (rs1 < static_cast<uint32_t>(imm)) ? 1u : 0u;
    break;
  default:
    throw UnimplementedInstructionException("I-Type funct3 not implemented.");
  }
  // throw UnimplementedInstructionException(
  //     "I‑type instructions not yet implemented");
  regs.write(d_instr.rd, rd);
  return current_pc + 4;
}

uint32_t execute_s_type(const DecodedInstruction & /*d_instr*/,
                        uint32_t /*current_pc*/, RegisterFile & /*regs*/,
                        Memory & /*mem*/) {
  throw UnimplementedInstructionException(
      "S‑type instructions not yet implemented");
}

uint32_t execute_b_type(const DecodedInstruction & /*d_instr*/,
                        uint32_t /*current_pc*/, RegisterFile & /*regs*/,
                        Memory & /*mem*/) {
  throw UnimplementedInstructionException(
      "B‑type instructions not yet implemented");
}

uint32_t execute_u_type(const DecodedInstruction & /*d_instr*/,
                        uint32_t /*current_pc*/, RegisterFile & /*regs*/,
                        Memory & /*mem*/) {
  throw UnimplementedInstructionException(
      "U‑type instructions not yet implemented");
}

uint32_t execute_j_type(const DecodedInstruction & /*d_instr*/,
                        uint32_t /*current_pc*/, RegisterFile & /*regs*/,
                        Memory & /*mem*/) {
  throw UnimplementedInstructionException(
      "J‑type instructions not yet implemented");
}

} // namespace execute
} // namespace riscv
