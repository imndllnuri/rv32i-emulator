import os
from PyQt5 import uic
from PyQt5.QtCore import Qt
from PyQt5.QtGui import QTextCursor, QTextDocument
from PyQt5.QtWidgets import QDialog, QTextEdit

UI_FILE = os.path.join(os.path.dirname(__file__), './ui/replace_dialog.ui')

class ReplaceDialog(QDialog):
    def __init__(self, text_edit, parent=None):
        super().__init__(parent)
        uic.loadUi(UI_FILE, self)

        self.text_edit = text_edit                # the QPlainTextEdit we search
        self.all_match_cursors = []                # stores all matches for counting & highlighting

        self.wrapAroundCheckBox.setChecked(True)   # Wrap around enabled by default
        self.caseSensitiveCheckBox.setChecked(True)
        self.highlightAllCheckBox.setChecked(True)

        self.findNextButton.clicked.connect(self.find_next)
        self.replaceButton.clicked.connect(self.replace_current)
        self.replaceAllButton.clicked.connect(self.replace_all)
        self.closeButton.clicked.connect(self.close)

        self.findLineEdit.textChanged.connect(self.find_all_matches)
        self.caseSensitiveCheckBox.toggled.connect(self.find_all_matches)
        self.wholeWordsCheckBox.toggled.connect(self.find_all_matches)
        self.highlightAllCheckBox.toggled.connect(self.update_highlighting)

        self.findNextButton.setEnabled(False)
        self.replaceButton.setEnabled(False)
        self.replaceAllButton.setEnabled(False)
        self.findLineEdit.textChanged.connect(self.update_buttons)
        self.findLineEdit.setPlaceholderText("Enter search term...")
        self.replaceWithLineEdit.textChanged.connect(self.update_buttons)
        self.replaceWithLineEdit.setPlaceholderText("Enter replace term...")
        self.statusLabel.setText("")

        cursor = self.text_edit.textCursor()
        if cursor.hasSelection():
            self.findLineEdit.setText(cursor.selectedText())

        # Initially compute matches (if any text was pre‑filled)
        self.find_all_matches()

    def update_buttons(self):
        enabled = bool(self.findLineEdit.text())
        self.findNextButton.setEnabled(enabled)
        self.replaceButton.setEnabled(enabled)
        self.replaceAllButton.setEnabled(enabled)

    def find_all_matches(self):
        term = self.findLineEdit.text()
        if not term:
            self.all_match_cursors = []
            self.update_highlighting()
            self._update_status_with_current()
            return

        flags = self._get_flags()
        matches = []
        cursor = QTextCursor(self.text_edit.document())
        cursor.movePosition(QTextCursor.Start)

        while True:
            cursor = self.text_edit.document().find(term, cursor, flags)
            if cursor.isNull():
                break
            # Store a copy of the cursor (find() reuses the same object)
            matches.append(QTextCursor(cursor))

        self.all_match_cursors = matches
        self.update_highlighting()
        self._update_status_with_current()

    def update_highlighting(self):
        if self.highlightAllCheckBox.isChecked() and self.all_match_cursors:
            extra_selections = []
            for cursor in self.all_match_cursors:
                selection = QTextEdit.ExtraSelection()
                selection.cursor = cursor
                selection.format.setBackground(Qt.GlobalColor.yellow)
                selection.format.setForeground(Qt.GlobalColor.black)
                extra_selections.append(selection)
            self.text_edit.setExtraSelections(extra_selections)
        else:
            self.text_edit.setExtraSelections([])

    def _update_status_with_current(self):
        if not self.findLineEdit.text():
            self.statusLabel.setText("")
            return

        total = len(self.all_match_cursors)
        if total == 0:
            self.statusLabel.setText("No matches")
            return

        current = self.text_edit.textCursor()

        # Find which stored match (if any) corresponds to the current cursor
        index = -1
        for i, match_cursor in enumerate(self.all_match_cursors):
            if (current.hasSelection() and
                current.selectionStart() == match_cursor.selectionStart() and
                current.selectionEnd() == match_cursor.selectionEnd()):
                index = i
                break

        if index >= 0:
            # Get line/column directly from the current cursor
            block = current.blockNumber() + 1
            column = current.columnNumber() + 1
            self.statusLabel.setText(
                f"Match {index+1} of {total} at line {block}, col {column}"
            )
        else:
            self.statusLabel.setText(f"{total} matches total")

    def _get_flags(self):
        flags = QTextDocument.FindFlags()
        if self.caseSensitiveCheckBox.isChecked():
            flags |= QTextDocument.FindFlag.FindCaseSensitively
        if self.wholeWordsCheckBox.isChecked():
            flags |= QTextDocument.FindFlag.FindWholeWords
        return flags

    def find_next(self):
        if not self.findLineEdit.text():
            return

        flags = self._get_flags()
        cursor = self.text_edit.textCursor()
        new_cursor = self.text_edit.document().find(
            self.findLineEdit.text(), cursor, flags
        )

        if new_cursor.isNull():
            if self.wrapAroundCheckBox.isChecked():
                cursor.movePosition(QTextCursor.Start)
                new_cursor = self.text_edit.document().find(
                    self.findLineEdit.text(), cursor, flags
                )
                if not new_cursor.isNull():
                    self.text_edit.setTextCursor(new_cursor)
                    self.statusLabel.setText("Wrapped to top")
                    self._update_status_with_current()
                else:
                    self.statusLabel.setText("No matches found")
            else:
                self.statusLabel.setText("No matches found")
        else:
            self.text_edit.setTextCursor(new_cursor)
            self._update_status_with_current()

    def replace_current(self):
        self.statusLabel.setText("replace current triggered")

    def replace_all(self):
        self.statusLabel.setText("replace all triggered")

    def closeEvent(self, event): # type: ignore 
        """Clear all highlights when the dialog is closed."""
        self.text_edit.setExtraSelections([])
        super().closeEvent(event)

