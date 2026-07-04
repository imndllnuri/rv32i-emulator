import os
from PyQt5 import uic
from PyQt5.QtCore import pyqtSignal
from PyQt5.QtWidgets import QWidget, QPlainTextEdit
from PyQt5.QtGui import QFont

from hexdump import format_hexdump

UI_FILE = os.path.join(os.path.dirname(__file__), "./ui/stack_window.ui")

SP_REG_INDEX = 2  # x2 = sp


class StackWidget(QWidget):
    """Dockable widget showing memory near the stack pointer."""

    addressChanged = pyqtSignal(int)

    def __init__(self, parent=None):
        super().__init__(parent)
        uic.loadUi(UI_FILE, self)

        self.cpu = None
        self.base_addr = 0
        self.bytes_per_line = 16
        self.bytesPerLine.setValue(self.bytes_per_line)
        self.bytesPerLine.setRange(1, 32)
        self.bytesPerLine.valueChanged.connect(self.set_bytes_per_line)
        self.display_lines = 16

        self.stackDisplay.setReadOnly(True)
        self.stackDisplay.setFont(QFont("Courier New", 10))
        self.stackDisplay.setLineWrapMode(QPlainTextEdit.NoWrap)

        self.goButton.clicked.connect(self.on_go_clicked)
        self.addressLineEdit.returnPressed.connect(self.on_go_clicked)
        self.refreshButton.clicked.connect(self.refresh)
        self.followSpCheckBox.toggled.connect(self.on_follow_sp_toggled)
        self.followSpCheckBox.setChecked(True)

        self.stackDisplay.setPlainText("No program loaded.")

    def set_cpu(self, cpu):
        self.cpu = cpu
        if cpu is not None:
            sp = cpu.get_registers()[SP_REG_INDEX]
            self.go_to_address(sp)

    def set_bytes_per_line(self, value):
        self.bytes_per_line = value
        self.refresh()

    def on_go_clicked(self):
        text = self.addressLineEdit.text().strip()
        if not text:
            return
        try:
            addr = int(text, 16) if text.lower().startswith("0x") else int(text)
        except ValueError:
            self.statusLabel.setText("Invalid address format")
            return
        self.go_to_address(addr)

    def go_to_address(self, addr):
        if self.cpu is None:
            self.stackDisplay.setPlainText("CPU not set.")
            return

        aligned = addr - (addr % self.bytes_per_line)
        self.base_addr = aligned
        self.addressLineEdit.setText(f"0x{aligned:08X}")

        try:
            data = self.cpu.read_memory(aligned, self.display_lines * self.bytes_per_line)
        except Exception as e:
            self.statusLabel.setText(f"Memory read error: {e}")
            self.stackDisplay.setPlainText("Error reading memory.")
            return

        hexdump = format_hexdump(data, aligned, self.bytes_per_line)
        self.stackDisplay.setPlainText(hexdump)
        self.statusLabel.setText(f"Showing stack at 0x{aligned:08X}")
        self.addressChanged.emit(aligned)

    def refresh(self):
        if self.cpu is not None:
            self.go_to_address(self.base_addr)

    def on_follow_sp_toggled(self, checked):
        if checked and self.cpu is not None:
            sp = self.cpu.get_registers()[SP_REG_INDEX]
            self.go_to_address(sp)

    def update_follow_sp(self, sp=None):
        """If follow SP is enabled, move the view to the current SP."""
        if self.followSpCheckBox.isChecked() and self.cpu is not None:
            if sp is None:
                sp = self.cpu.get_registers()[SP_REG_INDEX]
            self.go_to_address(sp)

    def set_controls_enabled(self, enabled):
        """Disable Go/Refresh while the CPU is running on a background
        thread, so a click here can't race the RunWorker's cpu.step()
        calls."""
        self.goButton.setEnabled(enabled)
        self.refreshButton.setEnabled(enabled)
