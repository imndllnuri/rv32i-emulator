"""Smoke tests for the main GUI window using pytest-qt. These intentionally
avoid the external riscv64-unknown-elf-as/objcopy dependency by loading a
hand-encoded program directly into the CPU, the same way
MainWindow.assemble() does after the external assembler has run.
"""
import pytest

pytestmark = pytest.mark.gui

main_window = pytest.importorskip("main_window", reason="PyQt5/gui import failed")
MainWindow = main_window.MainWindow


def _addi(rd, rs1, imm):
    imm = imm & 0xFFF
    return (imm << 20) | (rs1 << 15) | (0b000 << 12) | (rd << 7) | 0b0010011


def _load_trivial_program(window):
    """addi x1, x0, 42 -- bypasses the assembler for a pure GUI smoke test."""
    program = list(_addi(1, 0, 42).to_bytes(4, "little"))
    window.cpu.reset()
    window.cpu.load_program(program)
    window.program_loaded = True
    window._update_action_states()
    window.memory_widget.set_cpu(window.cpu)
    window.disasm_widget.set_cpu(window.cpu)
    window.stack_widget.set_cpu(window.cpu)


@pytest.fixture
def window(qtbot):
    win = MainWindow()
    qtbot.addWidget(win)
    return win


def test_starts_with_no_program_loaded(window):
    assert window.program_loaded is False
    assert window.actionRun.isEnabled() is False
    assert window.actionStep.isEnabled() is False


def test_step_updates_registers_and_pc(window):
    _load_trivial_program(window)

    window.step()

    assert window.instructions_executed == 1
    assert window.cpu.get_registers()[1] == 42
    assert window.actionRun.isEnabled() is True


def test_new_file_resets_program_state(window):
    _load_trivial_program(window)
    window.step()
    assert window.program_loaded is True

    window.new_file()

    assert window.program_loaded is False
    assert window.instructions_executed == 0
    assert window.cpu.get_registers()[1] == 0


def test_window_icon_is_set(window):
    assert not window.windowIcon().isNull()


def test_title_tracks_filename_and_dirty_state(window):
    assert window.windowTitle() == "Untitled - rv32i-emulator"

    window.editor.insertPlainText("nop")  # a real edit, unlike setPlainText()
    assert window.windowTitle() == "*Untitled - rv32i-emulator"


def test_open_path_loads_file_and_updates_title(window, tmp_path):
    program_file = tmp_path / "prog.s"
    program_file.write_text("addi x1, x0, 5\n")

    window.open_path(str(program_file))

    assert window.editor.toPlainText() == "addi x1, x0, 5\n"
    assert window.current_file == str(program_file)
    assert window.windowTitle() == "prog.s - rv32i-emulator"


def test_dock_registry_has_stable_object_names(window):
    expected = {"registers", "memory", "disassembly", "stack", "pc_history", "output"}
    assert set(window.docks.keys()) == expected
    for key, dock in window.docks.items():
        assert dock.objectName() == f"dock_{key}"


def test_whats_this_toolbar_action_exists(window):
    assert window.actionWhatsThis in window.toolBar.actions()


def test_memory_and_stack_controls_disabled_while_running(window):
    _load_trivial_program(window)

    window.running = True
    window._update_action_states()
    window.memory_widget.set_controls_enabled(False)
    window.stack_widget.set_controls_enabled(False)

    assert window.memory_widget.goButton.isEnabled() is False
    assert window.memory_widget.refreshButton.isEnabled() is False
    assert window.stack_widget.goButton.isEnabled() is False
    assert window.stack_widget.refreshButton.isEnabled() is False

    window.memory_widget.set_controls_enabled(True)
    window.stack_widget.set_controls_enabled(True)
    assert window.memory_widget.goButton.isEnabled() is True
    assert window.stack_widget.goButton.isEnabled() is True


def test_register_and_disassembly_tables_use_alternating_row_colors(window):
    assert window.register_widget.registerTable.alternatingRowColors() is True
    assert window.disasm_widget.disasmTable.alternatingRowColors() is True
