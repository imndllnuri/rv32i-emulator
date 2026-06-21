"""Pure-Python disassembler for the RV32I/RV32M + FENCE/CSR subset that
core/src/execute.cpp implements. Decoding mirrors the bit layout documented
in core/include/instruction.hpp - this is display-only logic, it does not
need to be byte-for-byte identical to a production disassembler.
"""

ABI_NAMES = [
    "zero", "ra", "sp", "gp", "tp", "t0", "t1", "t2",
    "s0", "s1", "a0", "a1", "a2", "a3", "a4", "a5",
    "a6", "a7", "s2", "s3", "s4", "s5", "s6", "s7",
    "s8", "s9", "s10", "s11", "t3", "t4", "t5", "t6",
]

OPCODE_R_TYPE = 0b0110011
OPCODE_I_TYPE = 0b0010011
OPCODE_I_TYPE_L = 0b0000011
OPCODE_I_TYPE_S = 0b0100011
OPCODE_B_TYPE = 0b1100011
OPCODE_JAL = 0b1101111
OPCODE_JALR = 0b1100111
OPCODE_LUI = 0b0110111
OPCODE_AUIPC = 0b0010111
OPCODE_SYSTEM = 0b1110011
OPCODE_FENCE = 0b0001111

_R_TYPE_MNEMONICS = {
    # (funct3, funct7): mnemonic
    (0b000, 0b0000000): "add",
    (0b000, 0b0100000): "sub",
    (0b000, 0b0000001): "mul",
    (0b001, 0b0000000): "sll",
    (0b001, 0b0000001): "mulh",
    (0b010, 0b0000000): "slt",
    (0b010, 0b0000001): "mulhsu",
    (0b011, 0b0000000): "sltu",
    (0b011, 0b0000001): "mulhu",
    (0b100, 0b0000000): "xor",
    (0b100, 0b0000001): "div",
    (0b101, 0b0000000): "srl",
    (0b101, 0b0100000): "sra",
    (0b101, 0b0000001): "divu",
    (0b110, 0b0000000): "or",
    (0b110, 0b0000001): "rem",
    (0b111, 0b0000000): "and",
    (0b111, 0b0000001): "remu",
}

_I_TYPE_MNEMONICS = {
    0b000: "addi",
    0b010: "slti",
    0b011: "sltiu",
    0b100: "xori",
    0b110: "ori",
    0b111: "andi",
    0b001: "slli",
    # 0b101 is shared by srli/srai, disambiguated via funct7 below
}

_LOAD_MNEMONICS = {0b000: "lb", 0b001: "lh", 0b010: "lw", 0b100: "lbu", 0b101: "lhu"}
_STORE_MNEMONICS = {0b000: "sb", 0b001: "sh", 0b010: "sw"}
_BRANCH_MNEMONICS = {
    0b000: "beq", 0b001: "bne", 0b100: "blt",
    0b101: "bge", 0b110: "bltu", 0b111: "bgeu",
}
_CSR_MNEMONICS = {
    0b001: "csrrw", 0b010: "csrrs", 0b011: "csrrc",
    0b101: "csrrwi", 0b110: "csrrsi", 0b111: "csrrci",
}

# All mnemonics this core understands, plus common pseudo-instructions
# (li, la, mv, j, ret, nop, call) that the GNU assembler expands - useful for
# the editor's syntax highlighter.
ALL_MNEMONICS = sorted(set(
    list(_R_TYPE_MNEMONICS.values())
    + list(_I_TYPE_MNEMONICS.values())
    + list(_LOAD_MNEMONICS.values())
    + list(_STORE_MNEMONICS.values())
    + list(_BRANCH_MNEMONICS.values())
    + list(_CSR_MNEMONICS.values())
    + ["jal", "jalr", "lui", "auipc", "ecall", "ebreak", "fence", "fence.i",
       "srli", "srai", "slli", "nop", "ret", "li", "la", "mv", "j", "call",
       "csrr", "csrw"]
))


def _reg(n):
    return ABI_NAMES[n & 0x1F]


def _sext(value, bits):
    mask = 1 << (bits - 1)
    return (value ^ mask) - mask


def _imm_i(word):
    return _sext(word >> 20, 12)


def _imm_s(word):
    imm = (((word >> 25) & 0x7F) << 5) | ((word >> 7) & 0x1F)
    return _sext(imm, 12)


def _imm_b(word):
    imm = (
        (((word >> 31) & 0x1) << 12)
        | (((word >> 7) & 0x1) << 11)
        | (((word >> 25) & 0x3F) << 5)
        | (((word >> 8) & 0xF) << 1)
    )
    return _sext(imm, 13)


def _imm_u(word):
    return word & 0xFFFFF000


def _imm_j(word):
    imm = (
        (((word >> 31) & 0x1) << 20)
        | (((word >> 12) & 0xFF) << 12)
        | (((word >> 20) & 0x1) << 11)
        | (((word >> 21) & 0x3FF) << 1)
    )
    return _sext(imm, 21)


def disassemble(word, pc=0):
    """Return a mnemonic + operand string for a 32-bit instruction word.

    `pc` is used to render absolute branch/jump targets when known.
    Returns "???" for anything not implemented by the core.
    """
    opcode = word & 0x7F
    rd = (word >> 7) & 0x1F
    funct3 = (word >> 12) & 0x7
    rs1 = (word >> 15) & 0x1F
    rs2 = (word >> 20) & 0x1F
    funct7 = (word >> 25) & 0x7F

    if opcode == OPCODE_R_TYPE:
        mnem = _R_TYPE_MNEMONICS.get((funct3, funct7))
        if mnem is None:
            return "???"
        return f"{mnem} {_reg(rd)}, {_reg(rs1)}, {_reg(rs2)}"

    if opcode == OPCODE_I_TYPE:
        imm = _imm_i(word)
        if funct3 == 0b101:
            mnem = "srai" if funct7 == 0b0100000 else "srli"
            return f"{mnem} {_reg(rd)}, {_reg(rs1)}, {imm & 0x1F}"
        if funct3 == 0b001:
            return f"slli {_reg(rd)}, {_reg(rs1)}, {imm & 0x1F}"
        mnem = _I_TYPE_MNEMONICS.get(funct3)
        if mnem is None:
            return "???"
        return f"{mnem} {_reg(rd)}, {_reg(rs1)}, {imm}"

    if opcode == OPCODE_I_TYPE_L:
        mnem = _LOAD_MNEMONICS.get(funct3)
        if mnem is None:
            return "???"
        imm = _imm_i(word)
        return f"{mnem} {_reg(rd)}, {imm}({_reg(rs1)})"

    if opcode == OPCODE_I_TYPE_S:
        mnem = _STORE_MNEMONICS.get(funct3)
        if mnem is None:
            return "???"
        imm = _imm_s(word)
        return f"{mnem} {_reg(rs2)}, {imm}({_reg(rs1)})"

    if opcode == OPCODE_B_TYPE:
        mnem = _BRANCH_MNEMONICS.get(funct3)
        if mnem is None:
            return "???"
        imm = _imm_b(word)
        target = (pc + imm) & 0xFFFFFFFF
        return f"{mnem} {_reg(rs1)}, {_reg(rs2)}, 0x{target:08X}"

    if opcode == OPCODE_JAL:
        imm = _imm_j(word)
        target = (pc + imm) & 0xFFFFFFFF
        return f"jal {_reg(rd)}, 0x{target:08X}"

    if opcode == OPCODE_JALR:
        imm = _imm_i(word)
        return f"jalr {_reg(rd)}, {imm}({_reg(rs1)})"

    if opcode == OPCODE_LUI:
        return f"lui {_reg(rd)}, 0x{_imm_u(word) >> 12:X}"

    if opcode == OPCODE_AUIPC:
        return f"auipc {_reg(rd)}, 0x{_imm_u(word) >> 12:X}"

    if opcode == OPCODE_FENCE:
        return "fence.i" if funct3 == 0b001 else "fence"

    if opcode == OPCODE_SYSTEM:
        if funct3 == 0:
            imm = _imm_i(word) & 0xFFF
            if imm == 0:
                return "ecall"
            if imm == 1:
                return "ebreak"
            return "???"
        mnem = _CSR_MNEMONICS.get(funct3)
        if mnem is None:
            return "???"
        csr_addr = (word >> 20) & 0xFFF
        if funct3 in (0b101, 0b110, 0b111):
            return f"{mnem} {_reg(rd)}, 0x{csr_addr:03X}, {rs1}"  # rs1 = zimm
        return f"{mnem} {_reg(rd)}, 0x{csr_addr:03X}, {_reg(rs1)}"

    return "???"
