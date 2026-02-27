import os
from PyQt5 import uic
from PyQt5.QtWidgets import QWidget, QTableWidgetItem, QHeaderView
from PyQt5.QtCore import pyqtSignal, Qt
from PyQt5.QtGui import QColor

UI_FILE = os.path.join(os.path.dirname(__file__), "./ui/register_window.ui")

# Colours used to highlight changed vs unchanged rows
HIGHLIGHT_BG  = QColor("#1565C0")   # blue – register changed this step
HIGHLIGHT_FG  = QColor("#FFFFFF")   # white text on blue
NORMAL_BG     = QColor(0, 0, 0, 0)  # transparent (let the theme show through)
NORMAL_FG     = QColor()            # invalid = use default palette colour

class RegisterWidget(QWidget):
    statusMessage = pyqtSignal(str, int)
    closeRequested = pyqtSignal()

    ABI_NAMES = [
        "zero", "ra", "sp", "gp", "tp", "t0", "t1", "t2",
        "s0", "s1", "a0", "a1", "a2", "a3", "a4", "a5",
        "a6", "a7", "s2", "s3", "s4", "s5", "s6", "s7",
        "s8", "s9", "s10", "s11", "t3", "t4", "t5", "t6"
    ]

    def __init__(self, parent=None):
        super().__init__(parent)
        uic.loadUi(UI_FILE, self)
        self._prev_values = [0] * 32   # track previous state for diffing
        self.setup_register_table()
        self.update_registers([0] * 32)

    def setup_register_table(self):
        self.registerTable.setColumnCount(4)
        self.registerTable.setHorizontalHeaderLabels(["Register", "ABI", "Hex", "Dec"])
        self.registerTable.verticalHeader().setVisible(False)
        header = self.registerTable.horizontalHeader()
        header.setSectionResizeMode(QHeaderView.Stretch)

    def update_registers(self, reg_values):
        """Refresh the table; highlight any register that changed since last call."""
        self.registerTable.setRowCount(32)
        for i in range(32):
            changed = (reg_values[i] != self._prev_values[i])

            name_item = QTableWidgetItem(f"X{i} - {self.ABI_NAMES[i]}")
            abi_item  = QTableWidgetItem(self.ABI_NAMES[i])
            hex_item  = QTableWidgetItem(f"0x{reg_values[i]:08X}")
            dec_item  = QTableWidgetItem(str(reg_values[i]))

            for item in (name_item, abi_item, hex_item, dec_item):
                if changed:
                    item.setBackground(HIGHLIGHT_BG)
                    item.setForeground(HIGHLIGHT_FG)
                else:
                    item.setData(Qt.BackgroundRole, None)
                    item.setData(Qt.ForegroundRole, None)

            self.registerTable.setItem(i, 0, name_item)
            self.registerTable.setItem(i, 1, abi_item)
            self.registerTable.setItem(i, 2, hex_item)
            self.registerTable.setItem(i, 3, dec_item)

        # Remember these values for the next diff
        self._prev_values = list(reg_values)
    
    def set_pc(self, pc):
        """Update the Program Counter display."""
        self.pcLineEdit.setText(f"0x{pc:08X}")

    def set_inst(self, inst_word):
        """Update the Current Instruction display (hex format)."""
        self.curInsLineEdit.setText(f"0x{inst_word:08X}")

    def clear_registers(self):
        self._prev_values = [0] * 32
        self.registerTable.setRowCount(0)
        self.pcLineEdit.clear()
        self.curInsLineEdit.clear()
