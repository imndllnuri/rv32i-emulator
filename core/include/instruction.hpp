// RV32I Base Instruction Set
// ---------------------------------------------------------------------
// LUI        | imm[31:12]                 | rd | 0110111
// AUIPC      | imm[31:12]                 | rd | 0010111
// JAL        | imm[20|10:1|11|19:12]      | rd | 1101111
// JALR       | imm[11:0]        | rs1|000 | rd | 1100111
//
// BEQ        | imm[12|10:5]| rs2| rs1|000 | imm[4:1|11] | 1100011
// BNE        | imm[12|10:5]| rs2| rs1|001 | imm[4:1|11] | 1100011
// BLT        | imm[12|10:5]| rs2| rs1|100 | imm[4:1|11] | 1100011
// BGE        | imm[12|10:5]| rs2| rs1|101 | imm[4:1|11] | 1100011
// BLTU       | imm[12|10:5]| rs2| rs1|110 | imm[4:1|11] | 1100011
// BGEU       | imm[12|10:5]| rs2| rs1|111 | imm[4:1|11] | 1100011
//
// LB         | imm[11:0]        | rs1|000 | rd | 0000011
// LH         | imm[11:0]        | rs1|001 | rd | 0000011
// LW         | imm[11:0]        | rs1|010 | rd | 0000011
// LBU        | imm[11:0]        | rs1|100 | rd | 0000011
// LHU        | imm[11:0]        | rs1|101 | rd | 0000011
//
// SB         | imm[11:5]  | rs2| rs1|000 | imm[4:0]   | 0100011
// SH         | imm[11:5]  | rs2| rs1|001 | imm[4:0]   | 0100011
// SW         | imm[11:5]  | rs2| rs1|010 | imm[4:0]   | 0100011
//
// ADDI       | imm[11:0]        | rs1|000 | rd | 0010011
// SLTI       | imm[11:0]        | rs1|010 | rd | 0010011
// SLTIU      | imm[11:0]        | rs1|011 | rd | 0010011
// XORI       | imm[11:0]        | rs1|100 | rd | 0010011
// ORI        | imm[11:0]        | rs1|110 | rd | 0010011
// ANDI       | imm[11:0]        | rs1|111 | rd | 0010011
// SLLI       | imm[11:0]        | rs1|001 | rd | 0010013
// SRLI       | imm[11:0]        | rs1|101 | rd | 0010013
// SRAI       | imm[11:0]        | rs1|101 | rd | 0010013
//
// ADD        | funct7=0000000| rs2| rs1|000 | rd | 0110011
// SUB        | funct7=0100000| rs2| rs1|000 | rd | 0110011
// SLL        | funct7=0000000| rs2| rs1|001 | rd | 0110011
// SLT        | funct7=0000000| rs2| rs1|010 | rd | 0110011
// SLTU       | funct7=0000000| rs2| rs1|011 | rd | 0110011
// XOR        | funct7=0000000| rs2| rs1|100 | rd | 0110011
// SRL        | funct7=0000000| rs2| rs1|101 | rd | 0110011
// SRA        | funct7=0100000| rs2| rs1|101 | rd | 0110011
// OR         | funct7=0000000| rs2| rs1|110 | rd | 0110011
// AND        | funct7=0000000| rs2| rs1|111 | rd | 0110011
//
// FENCE      | fm[3:0]|pred[3:0]|succ[3:0]| rs1|000 | rd | 0001111
// FENCE.I    | imm[11:0]        | rs1|001 | rd | 0001111
// ECALL      | imm[11:0]        | 000|000 | 000 | 1110011
// EBREAK     | imm[11:0]        | 000|001 | 000 | 1110011

#ifndef RISCV_INSTRUCTION_HPP
#define RISCV_INSTRUCTION_HPP

#include <cmath>
#include <cstdint>
#include <sys/types.h>

namespace riscv {

// Opcode values (7-bit)
namespace opcode {

// Integer Register - Register Operations
// ADD / SUB, SLL / SRL / SRA, AND / OR / XOR, SLT / SLTU
constexpr uint8_t R_TYPE = 0b0110011; // R-Type (ADD, SUB, AND, OR ..)
constexpr uint8_t I_TYPE = 0b0010011; // I-TYPE

} // namespace opcode

// funct3 values (3-bit)
namespace funct3 {
// R-Type funct3
constexpr uint8_t ADD_SUB = 0b000; // ADD, SUB, ADDI, ... d
constexpr uint8_t SLL = 0b001;     // d
constexpr uint8_t SLT = 0b010;
constexpr uint8_t SLTU = 0b011;
constexpr uint8_t XOR = 0b100;     // d
constexpr uint8_t SRL_SRA = 0b101; // SRA is msb-extends /d
constexpr uint8_t OR = 0b110;      // d
constexpr uint8_t AND = 0b111;     // d
// R-Type funct3

// I-Type funct3
constexpr uint8_t ADDI = 0b000;
constexpr uint8_t XORI = 0b100;
constexpr uint8_t ORI = 0b110;
constexpr uint8_t ANDI = 0b111;
constexpr uint8_t SLLI = 0b001;
constexpr uint8_t SRLI_SRAI = 0b101;
constexpr uint8_t SLTI = 0b010;
constexpr uint8_t SLTIU = 0b011;
// I-Type funct3

} // namespace funct3

// funct7 values (7-bit)
namespace funct7 {

constexpr uint8_t BASE =
    0b0000000; // 0x00 ADD, XOR, OR, AND, Shift Left Logical, Shift Right
               // Logical, set less than
constexpr uint8_t SUB_SRA = 0b0100000; // 0x20, shift right arith* (msb-extends)

// For RV32M (multiplication extension) you would add more here
} // namespace funct7

// Optional: helper to identify instruction type from opcode
// etc.

} // namespace riscv

#endif
