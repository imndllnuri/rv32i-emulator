import os
import sys
import subprocess
import tempfile

from PyQt5 import uic
from PyQt5.QtCore import Qt
from PyQt5.QtGui import QTextCursor, QFont 
from PyQt5.QtWidgets import QDockWidget, QFileDialog, QMainWindow, QMessageBox 

from find_dialog import FindDialog
from register_window import RegisterWidget
from memory_window import MemoryWidget
from replace_dialog import ReplaceDialog

# Add the path to the compiled pybind11 module
module_path = os.path.abspath(os.path.join(os.path.dirname(__file__), "../core/build"))
if module_path not in sys.path:
    sys.path.insert(0, module_path)

import rv32i_core

from PyQt5.QtCore import QThread, pyqtSignal

class RunWorker(QThread):
    # Emits the new register snapshot after each step
    step_done = pyqtSignal(list, int, int)
    finished = pyqtSignal()
    error = pyqtSignal(str)

    # How many instructions to execute before yielding to the GUI.
    # Larger = faster execution, smaller = more responsive UI updates.
    BATCH_SIZE = 100

    def __init__(self, cpu, max_steps=10_000_000, parent=None):
        super().__init__(parent)
        self.cpu = cpu
        self._is_running = True
        # Safety cap so an infinite loop (j halt) doesn't spin forever.
        # Set to 0 to disable the cap.
        self.max_steps = max_steps

    def stop(self):
        self._is_running = False

    def run(self):
        steps = 0
        while self._is_running:
            if self.max_steps and steps >= self.max_steps:
                break
            try:
                success = self.cpu.step()
                steps += 1
                if not success:
                    break
                if steps % self.BATCH_SIZE == 0:
                    regs = list(self.cpu.get_registers())
                    pc = self.cpu.get_pc()
                    # Read instruction word at PC (little‑endian)
                    try:
                        inst_bytes = self.cpu.read_memory(pc, 4)
                        inst = int.from_bytes(inst_bytes, byteorder='little')
                    except Exception:
                        inst = 0
                    self.step_done.emit(regs, pc, inst)
            except Exception as e:
                self.error.emit(str(e))
                break

        # Final snapshot
        regs = list(self.cpu.get_registers())
        pc = self.cpu.get_pc()
        try:
            inst_bytes = self.cpu.read_memory(pc, 4)
            inst = int.from_bytes(inst_bytes, byteorder='little')
        except Exception:
            inst = 0
        self.step_done.emit(regs, pc, inst)
        self.finished.emit()

UI_FILE = os.path.join(os.path.dirname(__file__), "./ui/main_window.ui")

class MainWindow(QMainWindow):
    def __init__(self):
        super().__init__()
        uic.loadUi(UI_FILE, self)

        self.current_file = None
        self.cpu = rv32i_core.CPU()
        self.running = False
        self.worker = None
        self.cpu.reset()
        self.program_loaded = False

        self.editor.setPlainText("Hello World!")
        # actions
        self.actionNew.triggered.connect(self.new_file)
        self.actionOpen.triggered.connect(self.open_file)
        self.actionRecent_Files.triggered.connect(
            lambda: self.statusbar.showMessage("Recent files triggered", 1000)
        )  # or open recent menu
        self.actionSave.triggered.connect(self.save_file)
        self.actionSave_as.triggered.connect(self.save_file_as)
        self.actionExit.triggered.connect(self.exit_app)

        self.actionUndo.triggered.connect(self.undo)
        self.actionRedo.triggered.connect(self.redo)
        self.actionCut.triggered.connect(self.cut)
        self.actionCopy.triggered.connect(self.copy)
        self.actionPaste.triggered.connect(self.paste)
        self.actionFind.triggered.connect(self.find_text)
        self.actionReplace.triggered.connect(self.replace)
        self.actionSelect_All.triggered.connect(self.select_all)

        self.actionShow_Tool_Bar.toggled.connect(self.toggle_toolbar)
        self.actionShow_Status_Bar.toggled.connect(self.toggle_statusbar)
        # self.actionShow_Registers.toggled.connect(self.toggle_registers)
        self.actionShow_Memory.toggled.connect(self.toggle_memory)
        self.actionShow_Output.toggled.connect(self.toggle_output)
        self.actionShow_Tool_Bar.setChecked(True)
        self.actionShow_Status_Bar.setChecked(True)

        self.actionZoom_In.triggered.connect(self.zoom_in)
        self.actionZoom_Out.triggered.connect(self.zoom_out)
        self.actionZoom_Reset.triggered.connect(self.zoom_reset)

        self.actionAssemble.triggered.connect(self.assemble)
        self.actionRun.triggered.connect(self.run)
        self.actionShow_Simulator.triggered.connect(self.show_simulator)
        self.actionSettings.triggered.connect(self.settings)

        self.actionHelp_Contents.triggered.connect(self.help_contents)
        self.actionAbout.triggered.connect(self.about)

        # self.actionToggle_Comment.triggered.connect(self.toggle_comment)
        # self.actionUntoggle_Comment.triggered.connect(self.untoggle_comment)

        self.actionStep.triggered.connect(self.step)
        # self.actionStep_Onto.triggered.connect(self.step_onto)
        # self.actionStep_Out.triggered.connect(self.step_out)
        
        # //register window dock
        self.register_dock = QDockWidget("Registers", self)
        self.register_widget = RegisterWidget(self)
        self.register_dock.setWidget(self.register_widget)
        self.addDockWidget(Qt.RightDockWidgetArea, self.register_dock)
        self.register_dock.setVisible(False)                # hidden by default

        # Forward status messages from register widget to main status bar
        self.register_widget.statusMessage.connect(self.statusbar.showMessage)

        # When the internal Close button is clicked, hide the dock
        self.register_widget.closeRequested.connect(self.register_dock.hide)

        # Synchronize menu action with dock visibility
        self.actionShow_Registers.toggled.connect(self.on_show_registers_toggled)
        self.register_dock.visibilityChanged.connect(self.actionShow_Registers.setChecked)

        # //memory window dock
        self.memory_dock = QDockWidget("Memory", self)
        self.memory_widget = MemoryWidget(self)
        self.memory_dock.setWidget(self.memory_widget)
        self.addDockWidget(Qt.RightDockWidgetArea, self.memory_dock)
        self.memory_dock.setVisible(False)

        self.actionShow_Memory.toggled.connect(self.memory_dock.setVisible)
        self.memory_dock.visibilityChanged.connect(self.actionShow_Memory.setChecked)
        # Connect the action

    def new_file(self):
        self.statusbar.showMessage("New file triggered", 1000)
        self.editor.clear()
        self.setWindowTitle("emulator-linux")
        self.editor.document().setModified(False)

    # TODO: implement multiple files opening
    def open_file(self):
        self.statusbar.showMessage("Open file triggered", 1000)

        # Create dialog with appropriate filters
        open_file_dialog = QFileDialog(self)
        open_file_dialog.setWindowTitle("Open File")
        open_file_dialog.setFileMode(QFileDialog.ExistingFile)
        open_file_dialog.setNameFilter(
            "Assembly Files (*.x68 *.asm);;Text Files (*.txt);;All Files (*)"
        )

        if open_file_dialog.exec():
            # selectedFiles() returns a list of paths; we take the first one
            file_path = open_file_dialog.selectedFiles()[0]
            self.statusbar.showMessage(f"Selected file: {file_path}", 3000)

            try:
                with open(file_path, "r") as f:
                    content = f.read()
                self.editor.setPlainText(content)
                self.current_file = file_path
                self.setWindowTitle(f"emulator-linux – {file_path}")
                self.statusbar.showMessage(f"Opened {file_path}", 3000)
                self.editor.document().setModified(False)
            except Exception as e:
                self.statusbar.showMessage(f"Error opening file: {str(e)}", 5000)
        else:
            self.statusbar.showMessage("Open cancelled.", 2000)

    def save_file(self):
        self.statusbar.showMessage("Save file triggered", 1000)
        if self.current_file:
            # we already have a file saved before we can save it.
            try:
                with open(self.current_file, "w") as f:
                    f.write(self.editor.toPlainText())
                self.statusbar.showMessage(f"Saved {self.current_file}", 3000)
                # we have just saved it so there is not any modification yet (clean state)
                self.editor.document().setModified(False)
            except Exception as e:
                self.statusbar.showMessage(f"Error saving: {str(e)}", 5000)
        else:
            # if self.current_file = false we should behave as save_file_as
            self.save_file_as()

    def save_file_as(self):
        self.statusbar.showMessage("Save as triggered", 1000)

        # Use a dialog that enforces a default suffix
        dialog = QFileDialog(self)
        dialog.setWindowTitle("Save File As")
        dialog.setNameFilter(
            "Assembly Files (*.x68);;Assembly Files (*.asm);;Text Files (*.txt);;All Files (*)"
        )
        dialog.setDefaultSuffix("x68")  # appends .x68 if no extension typed
        dialog.setAcceptMode(QFileDialog.AcceptSave)

        if dialog.exec():
            file_path = dialog.selectedFiles()[0]
            self.current_file = file_path
            try:
                with open(file_path, "w") as f:
                    f.write(self.editor.toPlainText())
                self.setWindowTitle(f"emulator-linux – {file_path}")
                self.statusbar.showMessage(f"Saved as {file_path}", 3000)
                self.editor.document().setModified(False)
            except Exception as e:
                self.statusbar.showMessage(f"Error saving: {str(e)}", 5000)
        else:
            self.statusbar.showMessage("Save cancelled.", 2000)

    def exit_app(self):
        self.statusbar.showMessage("Exit triggered :(", 1000)
        self.close()

    def undo(self):
        self.statusbar.showMessage("Undo triggered", 1000)
        self.editor.undo()

    def redo(self):
        self.statusbar.showMessage("Redo triggered", 1000)
        self.editor.redo()
    def cut(self):
        self.statusbar.showMessage("Cut triggered", 1000)
        self.editor.cut()

    def copy(self):
        self.statusbar.showMessage("Copy triggered", 1000)
        self.editor.copy()

    def paste(self):
        self.statusbar.showMessage("Paste triggered", 1000)
        self.editor.paste()

    def find_text(self):
        self.statusbar.showMessage("Find triggered", 1000)
        find_dialog = FindDialog(self.editor, self)
        find_dialog.exec()

    def replace(self):
        self.statusbar.showMessage("Replace triggered", 1000)
        replace_dialog = ReplaceDialog(self.editor, self)
        replace_dialog.exec()
        # TODO: open replace dialog

    def select_all(self):
        self.statusbar.showMessage("Select All triggered", 1000)
        self.editor.selectAll()

    def toggle_toolbar(self, checked):
        self.statusbar.showMessage(f"Toolbar toggled: {checked}", 1000)
        self.toolBar.setVisible(checked)

    def toggle_statusbar(self, checked):
        self.statusbar.showMessage(f"Status bar toggled: {checked}", 1000)
        self.statusbar.setVisible(checked)

    def on_show_registers_toggled(self, checked):
        """Show or hide the register dock when the menu action is toggled."""
        self.statusbar.showMessage(f"Registers window toggled: {checked}", 1000)
        self.register_dock.setVisible(checked)

    def toggle_memory(self, checked):
        self.statusbar.showMessage(f"Memory view toggled: {checked}", 1000)
        self.memory_dock.setVisible(checked)

    def toggle_output(self, checked):
        self.statusbar.showMessage(f"Output window toggled: {checked}", 1000)
        # TODO: show/hide output dock

    def zoom_in(self):
        self.statusbar.showMessage("Zoom In triggered", 1000)
        font = self.editor.font()
        font.setPointSize(font.pointSize() + 1)
        self.editor.setFont(font)

    def zoom_out(self):
        self.statusbar.showMessage("Zoom Out triggered", 1000)
        font = self.editor.font()
        if font.pointSize() > 1:
            font.setPointSize(font.pointSize() - 1)
            self.editor.setFont(font)

    def zoom_reset(self):
        self.statusbar.showMessage("Zoom Reset triggered", 1000)
        font = QFont("Courier New", 10)
        font.setFixedPitch(True)
        self.editor.setFont(font)

    def update_register_display(self, reg_values=None, pc=None, inst=None):
        """Update register dock, PC, and current instruction."""
        if not self.program_loaded:
            return

        # Update register table
        if reg_values is not None:
            self.register_widget.update_registers(reg_values)
        else:
            # Called from step() – read CPU directly
            reg_values = list(self.cpu.get_registers())
            self.register_widget.update_registers(reg_values)

        # Update PC
        if pc is not None:
            self.register_widget.set_pc(pc)
        else:
            pc = self.cpu.get_pc()
            self.register_widget.set_pc(pc)

        # Update current instruction
        if inst is not None:
            self.register_widget.set_inst(inst)
        else:
            # Fallback: read from CPU (only safe when not running)
            try:
                inst_bytes = self.cpu.read_memory(pc, 4)
                inst = int.from_bytes(inst_bytes, byteorder='little')
            except Exception:
                inst = 0
            self.register_widget.set_inst(inst)

        # Keep other views in sync
        if hasattr(self, 'memory_widget'):
            self.memory_widget.update_follow_pc(pc)
        if hasattr(self, 'programCounterEdit'):
            self.programCounterEdit.setText(f"0x{pc:08X}")

    def assemble(self):
        # 1. Check if there is a current file (saved)
        if not self.current_file:
            # No file saved yet – prompt to save as first
            reply = QMessageBox.question(
                self,
                "Save File",
                "You need to save the assembly file before assembling.\nSave now?",
                QMessageBox.Yes | QMessageBox.No
            )
            if reply == QMessageBox.Yes:
                self.save_file_as()          # this will set self.current_file if successful
                if not self.current_file:    # user cancelled or error
                    return
            else:
                self.statusbar.showMessage("Assembly cancelled.", 3000)
                return

        # 2. Determine output binary path (same directory, same base name, .bin extension)
        base, _ = os.path.splitext(self.current_file)
        bin_file = base + ".bin"

        # 3. Write current editor content to the existing .s/.asm file (overwrite)
        try:
            with open(self.current_file, "w") as f:
                f.write(self.editor.toPlainText())
        except Exception as e:
            self.statusbar.showMessage(f"Error writing assembly file: {e}", 5000)
            return

        # 4. Assemble using riscv64-unknown-elf-as and objcopy
        try:
            # Step 4a: assemble to ELF (temporary file, because we only need binary)
            # We'll assemble directly to a temporary ELF and then objcopy to bin.
            import tempfile
            with tempfile.NamedTemporaryFile(suffix='.elf', delete=False) as tmp_elf:
                elf_path = tmp_elf.name

            subprocess.run(
                ["riscv64-unknown-elf-as", "-march=rv32im", "-o", elf_path, self.current_file],
                check=True, capture_output=True, text=True
            )

            # Step 4b: convert ELF to binary at desired location
            subprocess.run(
                ["riscv64-unknown-elf-objcopy", "-O", "binary", elf_path, bin_file],
                check=True, capture_output=True, text=True
            )

            # Clean up temporary ELF
            os.unlink(elf_path)

        except subprocess.CalledProcessError as e:
            self.statusbar.showMessage(f"Assembly failed: {e.stderr}", 5000)
            return
        except FileNotFoundError as e:
            self.statusbar.showMessage(
                "Toolchain not found. Ensure 'riscv64-unknown-elf-as' and 'objcopy' are in PATH.",
                5000
            )
            return

        # 5. Load the binary into the CPU
        try:
            with open(bin_file, "rb") as f:
                program_bytes = f.read()
            self.cpu.reset()
            self.cpu.load_program(list(program_bytes))   # load at default TEXT_START
            self.program_loaded = True
            # // giving cpu reference to the memory view
            self.memory_widget.set_cpu(self.cpu) 
            self.statusbar.showMessage(
                f"Assembled and loaded {os.path.basename(bin_file)} successfully.",
                3000
            )
            self.update_register_display()   # reset registers to zero
        except Exception as e:
            self.statusbar.showMessage(f"Failed to load binary into CPU: {e}", 5000)

    def run(self):
        if not self.program_loaded:
            self.statusbar.showMessage("No program loaded. Assemble first.", 3000)
            return
        # If already running, clicking Run again stops it (toggle behaviour)
        if self.running:
            self.worker.stop()
            return

        self.running = True
        self.actionRun.setText("Stop")          # visual feedback
        self.actionStep.setEnabled(False)  # disable stepping while running

        self.worker = RunWorker(self.cpu, max_steps=10_000_000)
        # step_done now carries the register snapshot, so wire it directly
        self.worker.step_done.connect(self.update_register_display)
        self.worker.finished.connect(self.on_run_finished)
        self.worker.error.connect(self.on_run_error)
        self.worker.start()
        self.statusbar.showMessage("Running…", 0)   # 0 = stay until replaced

    def on_run_finished(self):
        self.running = False
        self.actionRun.setText("Run")
        self.actionStep.setEnabled(True)
        self.worker = None
        # Final register refresh (reads PC too)
        self.update_register_display()
        self.statusbar.showMessage("CPU halted.", 3000)

    def on_run_error(self, msg):
        self.statusbar.showMessage(f"Run error: {msg}", 5000)
        self.on_run_finished()

    def step(self):
        if not self.program_loaded:
            self.statusbar.showMessage("No program loaded. Assemble first.", 3000)
            return
        if self.running:
            self.statusbar.showMessage("Stop the running program first.", 2000)
            return
        try:
            success = self.cpu.step()
            self.update_register_display()          # refresh table + PC
            if not success:
                self.statusbar.showMessage("CPU halted.", 3000)
            else:
                pc = self.cpu.get_pc()
                self.statusbar.showMessage(f"Stepped → PC = 0x{pc:08X}", 2000)
        except Exception as e:
            self.statusbar.showMessage(f"Step error: {e}", 5000)

    def show_simulator(self):
        self.statusbar.showMessage("Show simulator triggered", 1000)
        # TODO: open simulator window

    def settings(self):
        self.statusbar.showMessage("Settings triggered", 1000)
        # TODO: open settings dialog

    def help_contents(self):
        self.statusbar.showMessage("Help contents triggered", 1000)
        # TODO: show help

    def about(self):
        self.statusbar.showMessage("About triggered", 1000)
        # TODO: show about dialog
