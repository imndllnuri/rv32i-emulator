// Shared instruction encoders for core/tests/test_*.cc.
//
// Every test file used to redefine its own `make_*_type()` helpers; this
// header is the single source of truth for how each format packs its bits,
// matching the field layout documented in core/include/instruction.hpp.
#ifndef RISCV_TESTS_INSTR_ENCODERS_HPP
#define RISCV_TESTS_INSTR_ENCODERS_HPP

#include <cstdint>

namespace rv32i {

inline uint32_t make_r_type(uint32_t funct7, uint32_t rs2, uint32_t rs1,
                            uint32_t funct3, uint32_t rd) {
  return (funct7 << 25) | (rs2 << 20) | (rs1 << 15) | (funct3 << 12) |
         (rd << 7) | 0b0110011;
}

inline uint32_t make_i_type(uint32_t funct3, uint32_t rs1, uint32_t rd,
                            int32_t imm) {
  uint32_t uimm = static_cast<uint32_t>(imm) & 0xFFF;
  return (uimm << 20) | (rs1 << 15) | (funct3 << 12) | (rd << 7) | 0b0010011;
}

inline uint32_t make_shift_i_type(uint32_t funct3, uint32_t funct7,
                                  uint32_t rs1, uint32_t rd,
                                  uint32_t shamt) {
  uint32_t imm = (funct7 << 5) | (shamt & 0x1F);
  return (imm << 20) | (rs1 << 15) | (funct3 << 12) | (rd << 7) | 0b0010011;
}

inline uint32_t make_load(uint32_t funct3, uint32_t rs1, uint32_t rd,
                          int32_t imm) {
  uint32_t uimm = static_cast<uint32_t>(imm) & 0xFFF;
  return (uimm << 20) | (rs1 << 15) | (funct3 << 12) | (rd << 7) |
         0b0000011; // LOAD opcode
}

inline uint32_t make_s_type(uint32_t funct3, uint32_t rs1, uint32_t rs2,
                            int32_t imm) {
  uint32_t uimm = static_cast<uint32_t>(imm);
  uint32_t imm11_5 = (uimm >> 5) & 0x7F;
  uint32_t imm4_0 = uimm & 0x1F;
  return (imm11_5 << 25) | (rs2 << 20) | (rs1 << 15) | (funct3 << 12) |
         (imm4_0 << 7) | 0b0100011; // STORE opcode
}

inline uint32_t make_b_type(uint32_t funct3, uint32_t rs1, uint32_t rs2,
                            int32_t imm) {
  uint32_t uimm = static_cast<uint32_t>(imm);
  uint32_t imm12 = (uimm >> 12) & 1;
  uint32_t imm11 = (uimm >> 11) & 1;
  uint32_t imm10_5 = (uimm >> 5) & 0x3F;
  uint32_t imm4_1 = (uimm >> 1) & 0xF;
  return (imm12 << 31) | (imm10_5 << 25) | (rs2 << 20) | (rs1 << 15) |
         (funct3 << 12) | (imm4_1 << 8) | (imm11 << 7) |
         0b1100011; // BRANCH opcode
}

inline uint32_t make_u_type(uint32_t opcode, uint32_t rd, int32_t imm) {
  uint32_t uimm = static_cast<uint32_t>(imm) & 0xFFFFF000;
  return uimm | (rd << 7) | opcode;
}

inline uint32_t make_jal(uint32_t rd, int32_t imm) {
  uint32_t uimm = static_cast<uint32_t>(imm);
  uint32_t imm20 = (uimm >> 20) & 1;
  uint32_t imm19_12 = (uimm >> 12) & 0xFF;
  uint32_t imm11 = (uimm >> 11) & 1;
  uint32_t imm10_1 = (uimm >> 1) & 0x3FF;
  return (imm20 << 31) | (imm10_1 << 21) | (imm11 << 20) | (imm19_12 << 12) |
         (rd << 7) | 0b1101111; // JAL opcode
}

inline uint32_t make_jalr(uint32_t rs1, uint32_t rd, int32_t imm) {
  uint32_t uimm = static_cast<uint32_t>(imm) & 0xFFF;
  return (uimm << 20) | (rs1 << 15) | (0x0 << 12) | (rd << 7) |
         0b1100111; // JALR opcode
}

// FENCE/FENCE.I share the I-type layout; pred/succ/fm bits in the immediate
// are irrelevant since this core has no caches or multiple harts.
inline uint32_t make_fence(uint32_t funct3) {
  return (funct3 << 12) | 0b0001111; // rd=0, rs1=0, imm=0
}

inline uint32_t make_ebreak() {
  return (1 << 20) | 0b1110011; // imm=1, rd=0, rs1=0, funct3=0
}

inline uint32_t make_csr(uint32_t csr_addr, uint32_t rs1, uint32_t funct3,
                         uint32_t rd) {
  return (csr_addr << 20) | (rs1 << 15) | (funct3 << 12) | (rd << 7) |
         0b1110011;
}

// For the *I variants, the rs1 field carries a 5-bit zero-extended immediate
// (zimm) instead of a register number.
inline uint32_t make_csri(uint32_t csr_addr, uint32_t zimm, uint32_t funct3,
                          uint32_t rd) {
  return (csr_addr << 20) | (zimm << 15) | (funct3 << 12) | (rd << 7) |
         0b1110011;
}

} // namespace rv32i

#endif
