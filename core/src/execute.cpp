#include "../include/execute.hpp"
#include "../include/decode.hpp"
#include "../include/exception.hpp" // your custom exceptions
#include "../include/instruction.hpp"
#include "../include/memory.hpp"
#include "../include/register.hpp"
#include <cstdint>
#include <sys/types.h>

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
                        RegisterFile &regs, Memory &mem) {
  if (d_instr.opcode != opcode::I_TYPE && d_instr.opcode != opcode::I_TYPE_L &&
      d_instr.opcode != opcode::JALR && d_instr.opcode != opcode::SYSTEM) {
    throw IllegalInstructionException("Non I-Type opcode in I-Type handler.");
  }
  uint32_t rs1 = regs.read(d_instr.rs1);
  uint32_t imm = d_instr.imm;
  uint32_t rd = 0;
  if (d_instr.opcode == opcode::I_TYPE_L) {
    uint32_t ea = rs1 + imm;
    switch (d_instr.funct3) {

    case funct3::LB:
      rd = static_cast<uint32_t>(
          static_cast<int32_t>(static_cast<int8_t>(mem.read_byte(ea))));
      break;
    case funct3::LH:
      rd = static_cast<uint32_t>(
          static_cast<int32_t>(static_cast<int16_t>(mem.read_half(ea))));
      break;
    case funct3::LW: // 0b010 - load word
      rd = mem.read_word(ea);
      break;
    case funct3::LBU: // 0b100 - load byte, zero-extend
      rd = mem.read_byte(ea);
      break;
    case funct3::LHU: // 0b101 - load half word, zero-extend
      rd = mem.read_half(ea);
      break;
    default:
      throw IllegalInstructionException("Invalid load funct3");
    }
  } else if (d_instr.opcode == opcode::I_TYPE) {

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
        throw IllegalInstructionException(
            "I-Type SRLI/SRAI with wrong funct7.");
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
  } else if (d_instr.opcode == opcode::JALR) {
    uint32_t rs1 = regs.read(d_instr.rs1);
    uint32_t target = (rs1 + imm) & ~1; // en düşük bit sıfırlanır
    regs.write(d_instr.rd, current_pc + 4);
    return target;
  } else if (d_instr.opcode == opcode::SYSTEM) {
    // ECALL ve EBREAK: funct3=0 should be
    if (d_instr.funct3 != 0) {
      throw IllegalInstructionException("Invalid SYSTEM funct3");
    }
    // imm değerine göre ayırt et
    if (d_instr.imm == 0) {
      // ECALL
      throw CpuException("ECALL encountered");
    } else if (d_instr.imm == 1) {
      // EBREAK
      throw CpuException("EBREAK encountered");
    } else {
      throw IllegalInstructionException("Unknown SYSTEM immediate");
    }
    // NOTE: after ECALL/EBREAK pc does not move, it creates trap. We can throw
    // exepction to mock behavior so step() returns false.
  } else {
    throw UnimplementedInstructionException(
        "I‑type instructions not yet implemented");
  }
  regs.write(d_instr.rd, rd);
  return current_pc + 4;
}
uint32_t execute_s_type(const DecodedInstruction &d_instr, uint32_t current_pc,
                        RegisterFile &regs, Memory &mem) {
  if (d_instr.opcode != opcode::I_TYPE_S) {
    throw IllegalInstructionException("Non S-Type opcode in S-Type handler.");
  }

  uint32_t rs1 = regs.read(d_instr.rs1);
  uint32_t rs2 = regs.read(d_instr.rs2);
  uint32_t addr = rs1 + d_instr.imm; // etkin adres

  switch (d_instr.funct3) {
  case funct3::SB:
    mem.write_byte(addr, static_cast<uint8_t>(rs2));
    break;
  case funct3::SH:
    mem.write_half(addr, static_cast<uint16_t>(rs2));
    break;
  case funct3::SW:
    mem.write_word(addr, rs2);
    break;
  default:
    throw IllegalInstructionException("S-Type funct3 not implemented.");
  }
  return current_pc + 4;
}

uint32_t execute_b_type(const DecodedInstruction &d_instr, uint32_t current_pc,
                        RegisterFile &regs, Memory & /*mem*/) {
  if (d_instr.opcode != opcode::B_TYPE) {
    throw IllegalInstructionException("Non B-Type opcode in B-Type handler.");
  }
  uint32_t rs1 = regs.read(d_instr.rs1);
  uint32_t rs2 = regs.read(d_instr.rs2);
  // the reason we have int32_t instead of uint32_t is that when there is a
  // backward branch if we have uint32_t it would break.
  int32_t imm = d_instr.imm;
  switch (d_instr.funct3) {
  case funct3::BEQ:
    return (rs1 == rs2) ? current_pc + imm : current_pc + 4;
  case funct3::BNE:
    return (rs1 != rs2) ? current_pc + imm : current_pc + 4;
  case funct3::BLT:
    return ((int32_t)rs1 < (int32_t)rs2) ? current_pc + imm : current_pc + 4;
  case funct3::BGE:
    return ((int32_t)rs1 >= (int32_t)rs2) ? current_pc + imm : current_pc + 4;
  case funct3::BLTU:
    return (rs1 < rs2) ? current_pc + imm : current_pc + 4;
  case funct3::BGEU:
    return (rs1 >= rs2) ? current_pc + imm : current_pc + 4;
  }
  throw UnimplementedInstructionException(
      "B‑type instructions not yet implemented");
}

uint32_t execute_u_type(const DecodedInstruction &d_instr, uint32_t current_pc,
                        RegisterFile &regs, Memory & /*mem*/) {
  uint32_t rd_val;
  if (d_instr.opcode == opcode::LUI) {
    // imm, imm_u() returned by it, 20-bit high value (bottom 12 bits are zero
    // anyways so we good.)
    rd_val = static_cast<uint32_t>(d_instr.imm);
  } else { // AUIPC
    rd_val = current_pc + static_cast<uint32_t>(d_instr.imm);
  }
  regs.write(d_instr.rd, rd_val);
  return current_pc + 4;
}

uint32_t execute_j_type(const DecodedInstruction &d_instr, uint32_t current_pc,
                        RegisterFile &regs, Memory &mem) {

  uint32_t next_pc;

  if (d_instr.opcode == opcode::JAL) {
    regs.write(d_instr.rd, current_pc + 4);
    next_pc = current_pc + d_instr.imm;
  } else {
    throw IllegalInstructionException("Invalid J-type opcode");
  }

  return next_pc;
}

} // namespace execute
} // namespace riscv
