import os
from PyQt5 import uic
from PyQt5.QtCore import Qt, pyqtSignal
from PyQt5.QtWidgets import QWidget, QPlainTextEdit
from PyQt5.QtGui import QFont, QColor, QTextCursor

UI_FILE = os.path.join(os.path.dirname(__file__), "./ui/memory_window.ui")

class MemoryWidget(QWidget):
    """A dockable widget for viewing memory contents in a hexdump style."""

    # Signal emitted when the user wants to jump to a specific address
    # (can be used by the main window to synchronise other views)
    addressChanged = pyqtSignal(int)

    def __init__(self, parent=None):
        super().__init__(parent)
        uic.loadUi(UI_FILE, self)

        # Store a reference to the CPU (set later via set_cpu())
        self.cpu = None

        # Current base address (aligned to bytes_per_line)
        self.base_addr = 0

        # Number of bytes displayed per line (default 16)
        self.bytes_per_line = 16
        self.bytesPerLine.setValue(self.bytes_per_line)
        self.bytesPerLine.valueChanged.connect(self.set_bytes_per_line)

        # Lines to display (we will fetch a fixed block, e.g. 256 bytes)
        self.display_lines = 16          # 16 lines * 16 bytes = 256 bytes

        # Set up the display widget
        self.memoryDisplay.setReadOnly(True)
        self.memoryDisplay.setFont(QFont("Courier New", 10))
        self.memoryDisplay.setLineWrapMode(QPlainTextEdit.NoWrap)

        # Connect signals
        self.goButton.clicked.connect(self.on_go_clicked)
        self.addressLineEdit.returnPressed.connect(self.on_go_clicked)
        self.refreshButton.clicked.connect(self.refresh)
        self.followPcCheckBox.toggled.connect(self.on_follow_pc_toggled)

        # Set placeholder texts
        self.addrLabel.setText("Address:")
        self.goButton.setText("Go")
        self.refreshButton.setText("Refresh")
        self.followPcCheckBox.setText("Follow PC")
        self.bytesPerLine.setSuffix(" bytes/line")
        self.bytesPerLine.setRange(1, 32)
        self.statusLabel.setText("Ready")

        # Initially empty display
        self.memoryDisplay.setPlainText("No program loaded.")

    def set_cpu(self, cpu):
        """Associate the CPU instance with this memory view."""
        self.cpu = cpu
        if cpu is not None:
            self.go_to_address(0)   # show start of memory

    def set_bytes_per_line(self, value):
        """Change the number of bytes shown per line and refresh."""
        self.bytes_per_line = value
        self.refresh()

    def on_go_clicked(self):
        """Parse the address from the line edit and jump there."""
        text = self.addressLineEdit.text().strip()
        if not text:
            return
        try:
            # Allow hex with or without 0x prefix
            if text.startswith("0x") or text.startswith("0X"):
                addr = int(text, 16)
            else:
                # Assume hex if it contains only hex digits, otherwise decimal?
                # For simplicity, treat as hex if it starts with digit and contains a-f
                if any(c in text.lower() for c in "abcdef"):
                    addr = int(text, 16)
                else:
                    addr = int(text)   # fallback to decimal
        except ValueError:
            self.statusLabel.setText("Invalid address format")
            return

        self.go_to_address(addr)

    def go_to_address(self, addr):
        """Display memory starting from a given address (aligned)."""
        if self.cpu is None:
            self.memoryDisplay.setPlainText("CPU not set.")
            return

        aligned = addr - (addr % self.bytes_per_line)
        self.base_addr = aligned
        self.addressLineEdit.setText(f"0x{aligned:08X}")

        try:
            # Use the new bulk read method
            data = self.cpu.read_memory(aligned, self.display_lines * self.bytes_per_line)
        except Exception as e:
            self.statusLabel.setText(f"Memory read error: {e}")
            self.memoryDisplay.setPlainText("Error reading memory.")
            return

        hexdump = self.format_hexdump(data, aligned)
        self.memoryDisplay.setPlainText(hexdump)
        self.statusLabel.setText(f"Showing address 0x{aligned:08X}")
        self.addressChanged.emit(aligned)

    def refresh(self):
        """Reload memory at the current base address."""
        if self.cpu is not None:
            self.go_to_address(self.base_addr)

    def format_hexdump(self, data, start_addr):
        """Convert a list of bytes into a traditional hexdump.

        Args:
            data: list of integers (0-255)
            start_addr: address of the first byte
        Returns:
            Multiline string: address + hex + ASCII
        """
        lines = []
        ascii_repr = []
        hex_bytes = []
        per_line = self.bytes_per_line

        for i, byte in enumerate(data):
            if i % per_line == 0 and i != 0:
                # Flush current line
                line_addr = start_addr + i - per_line
                line = f"{line_addr:08X}:  {' '.join(hex_bytes):<{per_line*3}}  |{''.join(ascii_repr)}|"
                lines.append(line)
                hex_bytes = []
                ascii_repr = []

            hex_bytes.append(f"{byte:02X}")
            # ASCII: printable characters only
            if 32 <= byte <= 126:
                ascii_repr.append(chr(byte))
            else:
                ascii_repr.append('.')

        # Last line (if any data left)
        if hex_bytes:
            line_addr = start_addr + len(data) - len(hex_bytes)
            # Pad hex part to full width for alignment
            hex_pad = ' '.join(hex_bytes)
            hex_padded = f"{hex_pad:<{per_line*3}}"
            line = f"{line_addr:08X}:  {hex_padded}  |{''.join(ascii_repr)}|"
            lines.append(line)

        return '\n'.join(lines)

    def on_follow_pc_toggled(self, checked):
        """Enable/disable automatic updates when PC changes."""
        if checked and self.cpu is not None:
            # Jump to current PC immediately
            pc = self.cpu.get_pc()
            self.go_to_address(pc)
        # The main window should call update_follow_pc() whenever PC changes

    def update_follow_pc(self, pc=None):
        """If follow PC is enabled, move the view to the current PC."""
        if self.followPcCheckBox.isChecked() and self.cpu is not None:
            if pc is None:
                pc = self.cpu.get_pc()
            self.go_to_address(pc)

    def closeEvent(self, event):
        """Override close to emit a signal if needed (the dock will hide)."""
        # Nothing special needed, but you could emit a signal if you want
        super().closeEvent(event)
