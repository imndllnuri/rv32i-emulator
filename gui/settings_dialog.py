"""Settings dialog wired to actionSettings. Built directly in code rather
than a .ui file since it's a small, self-contained dialog with no
Designer-specific layout needs.
"""
from PyQt5.QtWidgets import (
    QDialog, QDialogButtonBox, QFormLayout, QHBoxLayout, QLabel,
    QMessageBox, QPushButton, QSpinBox, QTabWidget, QVBoxLayout, QWidget,
)


class SettingsDialog(QDialog):
    """Editor preferences and debug-panel layout controls. Kept to what's
    actually functional today -- no fake "Appearance" tab, since the app
    only has a single hardcoded dark theme right now (see the project
    roadmap for theme configurability, deferred past beta-1)."""

    def __init__(self, main_window, parent=None):
        super().__init__(parent)
        self.main_window = main_window
        self.setWindowTitle("Settings")
        self.setWhatsThis("Editor and layout preferences for this application.")

        tabs = QTabWidget(self)
        tabs.addTab(self._build_editor_tab(), "Editor")
        tabs.addTab(self._build_layout_tab(), "Layout")

        buttons = QDialogButtonBox(QDialogButtonBox.Close, self)
        buttons.rejected.connect(self.close)
        buttons.button(QDialogButtonBox.Close).clicked.connect(self.close)

        layout = QVBoxLayout(self)
        layout.addWidget(tabs)
        layout.addWidget(buttons)

    def _build_editor_tab(self):
        tab = QWidget(self)
        form = QFormLayout(tab)

        self.font_size_spin = QSpinBox(tab)
        self.font_size_spin.setRange(6, 48)
        self.font_size_spin.setValue(self.main_window.editor.font().pointSize())
        self.font_size_spin.setWhatsThis(
            "Sets the code editor's font size. Takes effect immediately and "
            "is remembered for next time."
        )
        self.font_size_spin.valueChanged.connect(self._on_font_size_changed)
        form.addRow("Editor font size:", self.font_size_spin)

        return tab

    def _build_layout_tab(self):
        tab = QWidget(self)
        layout = QVBoxLayout(tab)

        info = QLabel(
            "Window size and the debug panel's split are saved automatically "
            "when you close the application."
        )
        info.setWordWrap(True)
        layout.addWidget(info)

        reset_row = QHBoxLayout()
        reset_button = QPushButton("Reset to Default Layout", tab)
        reset_button.setWhatsThis(
            "Restores the default editor/debug-panel split and reopens the "
            "panel if it's hidden, in case the layout has gotten into an "
            "unusable state."
        )
        reset_button.clicked.connect(self._on_reset_layout)
        reset_row.addWidget(reset_button)
        reset_row.addStretch(1)
        layout.addLayout(reset_row)
        layout.addStretch(1)

        return tab

    def _on_font_size_changed(self, size):
        font = self.main_window.editor.font()
        font.setPointSize(size)
        self.main_window.editor.setFont(font)
        self.main_window.app_settings.set_editor_font_size(size)

    def _on_reset_layout(self):
        reply = QMessageBox.question(
            self, "Reset Layout",
            "Reset the debug panel to its default layout?",
            QMessageBox.Yes | QMessageBox.No,
        )
        if reply == QMessageBox.Yes:
            self.main_window.reset_to_default_layout()
