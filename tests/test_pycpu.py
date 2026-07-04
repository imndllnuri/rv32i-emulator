"""Exercises the rv32i_core pybind11 module directly, independent of the GUI.
Requires core/ to have been built first (see CONTRIBUTING.md) -- skips
cleanly rather than erroring if the compiled extension isn't there yet.
"""
import pytest

rv32i_core = pytest.importorskip(
    "rv32i_core", reason="core/ hasn't been built yet -- see CONTRIBUTING.md"
)

TEXT_START = 0x1000


def make_addi(rd, rs1, imm):
    imm = imm & 0xFFF
    return (imm << 20) | (rs1 << 15) | (0b000 << 12) | (rd << 7) | 0b0010011


@pytest.fixture
def cpu():
    return rv32i_core.CPU()


SP_REG = 2
STACK_TOP = 0xEFFFF


def test_reset_initializes_registers(cpu):
    cpu.reset()
    regs = list(cpu.get_registers())
    # reset() deliberately seeds sp with STACK_TOP; every other register,
    # including x0, comes up zeroed.
    expected = [0] * 32
    expected[SP_REG] = STACK_TOP
    assert regs == expected
    assert cpu.get_pc() == TEXT_START


def test_step_executes_addi(cpu):
    cpu.reset()
    cpu.load_program(list(make_addi(1, 0, 42).to_bytes(4, "little")))
    assert cpu.step() is True
    assert cpu.get_registers()[1] == 42
    assert cpu.get_pc() == TEXT_START + 4


def test_x0_is_hardwired_zero(cpu):
    cpu.reset()
    cpu.load_program(list(make_addi(0, 0, 5).to_bytes(4, "little")))
    cpu.step()
    assert cpu.get_registers()[0] == 0


def test_memory_round_trip(cpu):
    cpu.reset()
    cpu.write_memory_word(TEXT_START + 0x100, 0xDEADBEEF)
    assert cpu.read_memory_word(TEXT_START + 0x100) == 0xDEADBEEF
    assert cpu.read_memory_byte(TEXT_START + 0x100) == 0xEF


def test_out_of_bounds_memory_read_raises(cpu):
    cpu.reset()
    with pytest.raises(Exception):
        cpu.read_memory_word(0xFFFFFFFF)


def test_step_returns_false_on_illegal_instruction(cpu):
    cpu.reset()
    cpu.load_program([0, 0, 0, 0])  # opcode 0 is not a valid RV32I opcode
    assert cpu.step() is False


def test_run_executes_until_illegal_instruction_halts_it(cpu):
    cpu.reset()
    program = list(make_addi(1, 0, 1).to_bytes(4, "little")) + [0, 0, 0, 0]
    cpu.load_program(program)
    cpu.run()
    assert cpu.get_registers()[1] == 1
    # The ADDI at TEXT_START executed and advanced PC; the illegal all-zero
    # word at TEXT_START+4 threw before PC could advance any further.
    assert cpu.get_pc() == TEXT_START + 4
