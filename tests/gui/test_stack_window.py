"""Regression tests for the Stack dock's current-SP line highlight, added
as part of the Phase 4 visual-polish work (previously the stack view was a
plain hex dump with no indication of where SP actually was).
"""
import pytest

pytestmark = pytest.mark.gui

rv32i_core = pytest.importorskip("rv32i_core", reason="core/ hasn't been built yet")
stack_window_module = pytest.importorskip("stack_window", reason="PyQt5/gui import failed")
StackWidget = stack_window_module.StackWidget


@pytest.fixture
def cpu():
    c = rv32i_core.CPU()
    c.reset()
    return c


def test_sp_line_is_highlighted_when_in_view(qtbot, cpu):
    widget = StackWidget()
    qtbot.addWidget(widget)
    widget.bytesPerLine.setValue(16)

    widget.set_cpu(cpu)

    selections = widget.stackDisplay.extraSelections()
    assert len(selections) == 1
    sp = cpu.get_registers()[2]
    line_base = widget.base_addr + ((sp - widget.base_addr) // 16) * 16
    assert f"{line_base:08X}" in selections[0].cursor.block().text().upper()


def test_highlight_clears_when_sp_scrolled_out_of_view(qtbot, cpu):
    widget = StackWidget()
    qtbot.addWidget(widget)
    widget.set_cpu(cpu)
    assert len(widget.stackDisplay.extraSelections()) == 1

    widget.go_to_address(0x2000)  # far from the initial sp

    assert widget.stackDisplay.extraSelections() == []


def test_highlight_updates_as_sp_moves_via_update_follow_sp(qtbot, cpu):
    widget = StackWidget()
    qtbot.addWidget(widget)
    widget.set_cpu(cpu)
    initial_sp = cpu.get_registers()[2]

    widget.update_follow_sp(initial_sp - 16)  # simulate a push

    assert widget._current_sp == initial_sp - 16
    assert len(widget.stackDisplay.extraSelections()) == 1
