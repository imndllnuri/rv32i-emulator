"""Regression tests for ReplaceDialog: replace_current()/replace_all() used
to be stubs that only set a status label and never touched the document.
Also covers that find_dialog.ui/replace_dialog.ui are no longer pinned to a
fixed size.
"""
import pytest

pytestmark = pytest.mark.gui

replace_dialog_module = pytest.importorskip("replace_dialog", reason="PyQt5/gui import failed")
find_dialog_module = pytest.importorskip("find_dialog", reason="PyQt5/gui import failed")
ReplaceDialog = replace_dialog_module.ReplaceDialog
FindDialog = find_dialog_module.FindDialog

from PyQt5.QtWidgets import QPlainTextEdit, QSizePolicy


@pytest.fixture
def editor(qtbot):
    edit = QPlainTextEdit()
    qtbot.addWidget(edit)
    return edit


def test_replace_all_replaces_every_match(editor, qtbot):
    editor.setPlainText("foo bar foo baz foo")
    dialog = ReplaceDialog(editor)
    qtbot.addWidget(dialog)
    dialog.findLineEdit.setText("foo")
    dialog.replaceWithLineEdit.setText("QUX")

    dialog.replace_all()

    assert editor.toPlainText() == "QUX bar QUX baz QUX"
    assert dialog.statusLabel.text() == "Replaced 3 occurrences"


def test_replace_current_arms_then_replaces(editor, qtbot):
    editor.setPlainText("foo bar foo")
    dialog = ReplaceDialog(editor)
    qtbot.addWidget(dialog)
    dialog.findLineEdit.setText("foo")
    dialog.replaceWithLineEdit.setText("X")

    dialog.replace_current()  # first click: selects the first match, no edit yet
    assert editor.toPlainText() == "foo bar foo"
    assert editor.textCursor().selectedText() == "foo"

    dialog.replace_current()  # second click: replaces the now-selected match
    assert editor.toPlainText() == "X bar foo"


def test_replace_all_is_a_no_op_on_empty_search_term(editor, qtbot):
    editor.setPlainText("foo bar foo")
    dialog = ReplaceDialog(editor)
    qtbot.addWidget(dialog)
    dialog.replaceWithLineEdit.setText("X")

    dialog.replace_all()

    assert editor.toPlainText() == "foo bar foo"


def test_find_and_replace_dialogs_are_resizable(editor, qtbot):
    find_dialog = FindDialog(editor)
    replace_dialog = ReplaceDialog(editor)
    qtbot.addWidget(find_dialog)
    qtbot.addWidget(replace_dialog)

    for dialog in (find_dialog, replace_dialog):
        assert dialog.maximumSize().width() >= 16777215
        assert dialog.maximumSize().height() >= 16777215
        assert dialog.sizePolicy().horizontalPolicy() != QSizePolicy.Fixed
