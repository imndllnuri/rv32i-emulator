"""Regression tests for gui/settings.py + the Settings dialog + dock-layout
persistence added in the Phase 4 roadmap work. QSettings storage is
redirected to a per-test temp dir by the autouse isolate_qsettings fixture
in tests/conftest.py.
"""
import pytest

main_window = pytest.importorskip("main_window", reason="PyQt5/gui import failed")
settings_dialog_module = pytest.importorskip("settings_dialog", reason="PyQt5/gui import failed")
MainWindow = main_window.MainWindow
SettingsDialog = settings_dialog_module.SettingsDialog


def test_font_size_and_geometry_persist_across_windows(qtbot):
    w1 = MainWindow()
    qtbot.addWidget(w1)
    w1.resize(1000, 700)
    w1.app_settings.set_editor_font_size(18)
    font = w1.editor.font()
    font.setPointSize(18)
    w1.editor.setFont(font)
    w1.close()  # closeEvent() saves window geometry/state

    w2 = MainWindow()
    qtbot.addWidget(w2)

    assert w2.editor.font().pointSize() == 18
    assert w2.geometry().width() == 1000
    assert w2.geometry().height() == 700


def test_recent_files_round_trip(qtbot):
    w1 = MainWindow()
    qtbot.addWidget(w1)
    w1.app_settings.add_recent_file("/tmp/a.s")
    w1.app_settings.add_recent_file("/tmp/b.s")

    w2 = MainWindow()
    qtbot.addWidget(w2)

    assert w2.app_settings.recent_files() == ["/tmp/b.s", "/tmp/a.s"]


def test_open_path_adds_to_recent_files(qtbot, tmp_path):
    window = MainWindow()
    qtbot.addWidget(window)
    program_file = tmp_path / "prog.s"
    program_file.write_text("nop\n")

    window.open_path(str(program_file))

    assert window.app_settings.recent_files() == [str(program_file)]


def test_settings_dialog_changes_font_live(qtbot):
    window = MainWindow()
    qtbot.addWidget(window)
    dialog = SettingsDialog(window)
    qtbot.addWidget(dialog)

    dialog.font_size_spin.setValue(22)

    assert window.editor.font().pointSize() == 22
    assert window.app_settings.editor_font_size() == 22


def test_reset_to_default_layout_shows_and_hides_expected_docks(qtbot):
    window = MainWindow()
    qtbot.addWidget(window)
    window.show()

    window.reset_to_default_layout()

    assert window.register_dock.isVisible()
    assert window.disasm_dock.isVisible()
    assert window.memory_dock.isVisible()
    assert not window.stack_dock.isVisible()
    assert not window.pc_history_dock.isVisible()
    assert not window.output_dock.isVisible()
