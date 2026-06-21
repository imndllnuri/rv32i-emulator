#include "../include/cpu.hpp"
#include "../include/decode.hpp"
#include "../include/exception.hpp"
#include "../include/execute.hpp"
#include "../include/fetch.hpp"
#include <cstdint>
#include <iostream>

namespace rv32i {

CPU::CPU() : pc(0), regs{}, mem{} { reset(); }

void CPU::reset() {
  regs.reset();
  csrs.reset();
  pc = TEXT_START;
  regs.write(static_cast<uint32_t>(Reg::sp), STACK_TOP);
  // memory is not cleared incase if we want to run the same program twice, we
  // dont have to load again
}

void CPU::load_program(const std::vector<uint8_t> &code, uint32_t address) {
  mem.load_program(code, address);
}

bool CPU::step() {
  try {
    uint32_t instr = fetch_instruction();
    uint32_t next_pc = execute_instruction(instr, pc);
    pc = next_pc;
    return true;
  } catch (const CpuException &e) {
    std::cerr << "CPU error at PC 0x" << std::hex << pc << ": " << e.what()
              << std::endl;
    return false;
  } catch (const MemoryAccessException &e) {
    std::cerr << "Memory error at PC 0x" << std::hex << pc << ": " << e.what()
              << std::endl;
    return false;
  }
}

uint32_t CPU::fetch_instruction() { return fetch::fetch_instruction(pc, mem); }

uint32_t CPU::execute_instruction(uint32_t instr, uint32_t current_pc) {
  DecodedInstruction d_instr = decode(instr);

  switch (d_instr.format) {
  case DecodedInstruction::Format::R:
    return execute::execute_r_type(d_instr, current_pc, regs, mem);
  case DecodedInstruction::Format::I:
    return execute::execute_i_type(d_instr, current_pc, regs, mem, csrs);
  case DecodedInstruction::Format::S:
    return execute::execute_s_type(d_instr, current_pc, regs, mem);
  case DecodedInstruction::Format::B:
    return execute::execute_b_type(d_instr, current_pc, regs, mem);
  case DecodedInstruction::Format::U:
    return execute::execute_u_type(d_instr, current_pc, regs, mem);
  case DecodedInstruction::Format::J:
    return execute::execute_j_type(d_instr, current_pc, regs, mem);
  default:
    throw UnimplementedInstructionException("Unknown instruction format");
  }
}

// Immediate helpers – simply forward to decode functions
// int32_t CPU::read_imm_i(uint32_t instr) const { return decode::imm_i(instr);
// } int32_t CPU::read_imm_s(uint32_t instr) const { return
// decode::imm_s(instr); } int32_t CPU::read_imm_b(uint32_t instr) const {
// return decode::imm_b(instr); } int32_t CPU::read_imm_u(uint32_t instr) const
// { return decode::imm_u(instr); } int32_t CPU::read_imm_j(uint32_t instr)
// const { return decode::imm_j(instr); }

void CPU::run() {
  uint64_t total_instr = 0;
  while (step()) {
    total_instr++;
  }
}

} // namespace rv32i
