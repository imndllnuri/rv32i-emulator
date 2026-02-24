import os

from PyQt5 import uic
from PyQt5.QtWidgets import QTableWidgetItem, QWidget 
from PyQt5.QtCore import pyqtSignal

UI_FILE = os.path.join(os.path.dirname(__file__), "./ui/register_window.ui")

class RegisterWidget(QWidget):
    statusMessage = pyqtSignal(str, int)
    closeRequested = pyqtSignal()
    def __init__(self, parent=None):
        super().__init__(parent)
        uic.loadUi(UI_FILE, self)

        self.create_register_table()

    def create_register_table(self):
        """Fill the register table with example data."""
        registers = [
            ("X0 - Zero", "data", "0x0000", "0", "00000000"),
            ("X1 - RA", "data", "0x0000", "0", "00000000"),
            ("X2 - SP", "address", "0x0000", "0", "00000000"),
        ]
        self.registerTable.setRowCount(len(registers))
        for row, (reg, abi, hex_val, dec_val, bin_val) in enumerate(registers):
            self.registerTable.setItem(row, 0, QTableWidgetItem(reg))
            self.registerTable.setItem(row, 1, QTableWidgetItem(abi))
            self.registerTable.setItem(row, 2, QTableWidgetItem(hex_val))
            self.registerTable.setItem(row, 3, QTableWidgetItem(dec_val))
            self.registerTable.setItem(row, 4, QTableWidgetItem(bin_val))

