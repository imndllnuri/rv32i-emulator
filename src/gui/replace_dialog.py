

import os
from PyQt5 import uic
from PyQt5.QtWidgets import QDialog

UI_FILE = os.path.join(os.path.dirname(__file__), 'replace_dialog.ui')

class ReplaceDialog(QDialog):
    def __init__(self, text_edit, parent=None):
        super().__init__(parent)
        uic.loadUi(UI_FILE, self)

        self.text_edit = text_edit                # the QPlainTextEdit we search
        self.all_match_cursors = []                # stores all matches for counting & highlighting

        self.wrapAroundCheckBox.setChecked(True)   # Wrap around enabled by default

        self.findNextButton.clicked.connect(self.find_next)
        self.replaceButton.clicked.connect(self.replace_current)
        self.replaceAllButton.clicked.connect(self.replace_all)
        self.closeButton.clicked.connect(self.close)

        self.replaceWithLineEdit.textChanged.connect(self.find_all_matches)
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
            pass

        def find_all_matches(self):
            pass

        def update_highlighting(self):
            pass

        def _update_status(self):
            pass

        def _get_flags(self):
            pass

        def find_next(self):
            pass

        def replace_current(self):
            pass

        def replace_all(self):
            pass

        def closeEvent(self, event):  # type: ignore
            pass

