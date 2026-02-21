#include "../include/cpu.hpp"
#include "../include/decode.hpp"
#include "../include/execute.hpp"
#include "../include/fetch.hpp"
#include "../include/instruction.hpp"
#include <cstdint>
#include <iostream>

namespace riscv {

CPU::CPU() : pc(0), regs{}, mem{} { reset(); }

void CPU::reset() {
  regs.reset();
  pc = TEXT_START;
  regs.write(static_cast<uint32_t>(Reg::sp), STACK_TOP);
  // memory is not cleared incase if we want to run the same program twice, we
  // dont have to load again
}

void CPU::load_program(const std::vector<uint8_t> &code, uint32_t address) {
  mem.load_program(code, address);
}

bool CPU::step() {
  uint32_t instr = fetch_instruction();
  uint32_t next_pc = execute_instruction(instr, pc);
  pc = next_pc;
  return true;
}

uint32_t CPU::fetch_instruction() { return fetch::fetch_instruction(pc, mem); }

uint32_t CPU::execute_instruction(uint32_t instr, uint32_t current_pc) {
  DecodedInstruction d_instr = decode(instr);

  switch (d_instr.format) {
  case DecodedInstruction::Format::R:
    return execute_r_type(d_instr, current_pc);
  case riscv::DecodedInstruction::Format::I:
    return execute_i_type(d_instr, current_pc);
  case riscv::DecodedInstruction::Format::S:
    return execute_s_type(d_instr, current_pc);
  case riscv::DecodedInstruction::Format::B:
    return execute_b_type(d_instr, current_pc);
  case riscv::DecodedInstruction::Format::U:
    return execute_u_type(d_instr, current_pc);
  case riscv::DecodedInstruction::Format::J:
    return execute_j_type(d_instr, current_pc);
  }
  return current_pc;
}

// Per‑type execution: all forward to the corresponding execute:: functions
uint32_t CPU::execute_i_type(const DecodedInstruction &d_instr,
                             uint32_t current_pc) {
  return execute::execute_i_type(d_instr, current_pc, regs, mem);
}

uint32_t CPU::execute_b_type(const DecodedInstruction &d_instr,
                             uint32_t current_pc) {
  return execute::execute_b_type(d_instr, current_pc, regs, mem);
}

uint32_t CPU::execute_r_type(const DecodedInstruction &d_instr,
                             uint32_t current_pc) {

  if (d_instr.opcode != opcode::IRRO) {
    // unkown opcode we can just advance for now (we'll raise an exception
    // later.)
    return current_pc + 4;
  }
  // funct3, imm, funct7, rs1, rs2, rd
  uint32_t rs1 = regs.read(d_instr.rs1);
  uint32_t rs2 = regs.read(d_instr.rs2);
  uint32_t rd;

  switch (d_instr.funct3) {
  case funct3::ADD_SUB: // 0b000
    if (d_instr.funct7 == funct7::BASE) {
      // add instruction
      rd = rs1 + rs2;
    } else if (d_instr.funct7 == funct7::SUB) {
      // sub instruction
      rd = rs1 - rs2;
    } else {
      // for now these are illegal instructions, we'll implement raising
      // exceptions
      return current_pc + 4;
    }
    break;
  case funct3::XOR:
    if (d_instr.funct7 == funct7::BASE) {
      rd = rs1 ^ rs2;
    } else {
      std::cout << "Illegal instruction. R-Type, XOR" << std::endl;
    }
    break;
  default:
    // unknown funct3 - just advance pc, we'll implement later.
    return current_pc + 4;
  }

  regs.write(d_instr.rd, rd);
  return current_pc + 4;
}

uint32_t CPU::execute_s_type(const DecodedInstruction &d_instr,
                             uint32_t current_pc) {
  return current_pc;
}
uint32_t CPU::execute_u_type(const DecodedInstruction &d_instr,
                             uint32_t current_pc) {
  return current_pc;
}
uint32_t CPU::execute_j_type(const DecodedInstruction &d_instr,
                             uint32_t current_pc) {
  return current_pc;
}

// Immediate helpers – simply forward to decode functions
// int32_t CPU::read_imm_i(uint32_t instr) const { return decode::imm_i(instr);
// } int32_t CPU::read_imm_s(uint32_t instr) const { return
// decode::imm_s(instr); } int32_t CPU::read_imm_b(uint32_t instr) const {
// return decode::imm_b(instr); } int32_t CPU::read_imm_u(uint32_t instr) const
// { return decode::imm_u(instr); } int32_t CPU::read_imm_j(uint32_t instr)
// const { return decode::imm_j(instr); }

void CPU::run() {
  // Not implemented in original – kept as is
}

} // namespace riscv
