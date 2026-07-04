import os
from PyQt5 import uic
from PyQt5.QtCore import pyqtSignal
from PyQt5.QtWidgets import QWidget, QTableWidgetItem, QHeaderView
from PyQt5.QtGui import QColor

from disassembler import disassemble

UI_FILE = os.path.join(os.path.dirname(__file__), "./ui/disassembly_window.ui")

PC_BG = QColor("#1565C0")
PC_FG = QColor("#FFFFFF")
BREAKPOINT_FG = QColor("#E53935")


class DisassemblyWidget(QWidget):
    """Dockable widget showing a window of disassembled instructions around
    the current PC. Double-clicking a row toggles a breakpoint on it."""

    breakpointsChanged = pyqtSignal(set)

    def __init__(self, parent=None):
        super().__init__(parent)
        uic.loadUi(UI_FILE, self)

        self.cpu = None
        self.base_addr = 0
        self.current_pc = 0
        self.instr_count = 32  # number of instructions shown at once
        self.breakpoints = set()
        self._row_addrs = []

        self.disasmTable.setColumnCount(4)
        self.disasmTable.setAlternatingRowColors(True)
        header = self.disasmTable.horizontalHeader()
        header.setSectionResizeMode(QHeaderView.ResizeToContents)

        self.goButton.clicked.connect(self.on_go_clicked)
        self.addressLineEdit.returnPressed.connect(self.on_go_clicked)
        self.refreshButton.clicked.connect(self.refresh)
        self.followPcCheckBox.toggled.connect(self.on_follow_pc_toggled)
        self.disasmTable.cellDoubleClicked.connect(self.on_cell_double_clicked)

        self.statusLabel.setText("No program loaded.")

    def set_cpu(self, cpu):
        self.cpu = cpu
        if cpu is not None:
            self.go_to_address(cpu.get_pc())

    def on_go_clicked(self):
        text = self.addressLineEdit.text().strip()
        if not text:
            return
        try:
            addr = int(text, 16)
        except ValueError:
            self.statusLabel.setText("Invalid address format")
            return
        self.go_to_address(addr)

    def go_to_address(self, addr):
        if self.cpu is None:
            self.statusLabel.setText("CPU not set.")
            return

        aligned = addr - (addr % 4)
        self.base_addr = aligned
        self.addressLineEdit.setText(f"0x{aligned:08X}")

        try:
            data = self.cpu.read_memory(aligned, self.instr_count * 4)
        except Exception as e:
            self.statusLabel.setText(f"Memory read error: {e}")
            return

        self._render(data, aligned)
        self.statusLabel.setText(f"Showing address 0x{aligned:08X}")

    def _render(self, data, start_addr):
        rows = len(data) // 4
        self.disasmTable.setRowCount(rows)
        self._row_addrs = []

        for i in range(rows):
            addr = start_addr + i * 4
            word = int.from_bytes(data[i * 4:i * 4 + 4], byteorder="little")
            mnemonic = disassemble(word, pc=addr)
            self._row_addrs.append(addr)

            bp_item = QTableWidgetItem("●" if addr in self.breakpoints else "")
            addr_item = QTableWidgetItem(f"0x{addr:08X}")
            bytes_item = QTableWidgetItem(f"{word:08X}")
            mnem_item = QTableWidgetItem(mnemonic)

            items = (bp_item, addr_item, bytes_item, mnem_item)
            is_pc = addr == self.current_pc
            is_bp = addr in self.breakpoints
            for item in items:
                if is_pc:
                    item.setBackground(PC_BG)
                    item.setForeground(PC_FG)
                elif is_bp:
                    item.setForeground(BREAKPOINT_FG)

            for col, item in enumerate(items):
                self.disasmTable.setItem(i, col, item)

    def refresh(self):
        if self.cpu is not None:
            self.go_to_address(self.base_addr)

    def set_pc(self, pc):
        self.current_pc = pc
        if self.followPcCheckBox.isChecked():
            self.go_to_address(pc)
        else:
            self.refresh()

    def on_follow_pc_toggled(self, checked):
        if checked and self.cpu is not None:
            self.go_to_address(self.current_pc)

    def on_cell_double_clicked(self, row, _col):
        if row < 0 or row >= len(self._row_addrs):
            return
        addr = self._row_addrs[row]
        if addr in self.breakpoints:
            self.breakpoints.discard(addr)
        else:
            self.breakpoints.add(addr)
        self.breakpointsChanged.emit(set(self.breakpoints))
        self.refresh()
