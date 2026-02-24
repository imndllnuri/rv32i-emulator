#ifndef RISCV_DECODE_HPP
#define RISCV_DECODE_HPP

#include <cstdint>

namespace rv32i {

// Compact decoded instruction – ready for execution
struct DecodedInstruction {
  enum class Format : uint8_t { R, I, S, B, U, J };

  Format format;  // which instruction format
  uint8_t opcode; // 7‑bit opcode
  uint8_t rd;     // destination register
  uint8_t rs1;    // source register 1
  uint8_t rs2;    // source register 2
  uint8_t funct3; // 3‑bit function code
  uint8_t funct7; // 7‑bit function code (only for R‑type)
  int32_t imm;    // sign‑extended immediate (ready to use)
};

// Decode a raw 32‑bit instruction into a DecodedInstruction struct
DecodedInstruction decode(uint32_t instr);

} // namespace rv32i

#endif
