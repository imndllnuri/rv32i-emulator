import glob
import os
import platform
import shutil
import sys
import subprocess
import tempfile
from collections import deque

from PyQt5 import uic
from PyQt5.QtCore import Qt
from PyQt5.QtGui import QTextCursor, QFont, QIcon, QCursor
from PyQt5.QtWidgets import (
    QDockWidget, QFileDialog, QMainWindow, QMessageBox, QListWidget,
    QPlainTextEdit, QStyle, QLabel, QWhatsThis, QMenu,
)

from find_dialog import FindDialog
from register_window import RegisterWidget
from memory_window import MemoryWidget
from replace_dialog import ReplaceDialog
from disassembly_window import DisassemblyWidget
from stack_window import StackWidget
from disassembler import OPCODE_JAL, OPCODE_JALR
from code_editor import CodeEditor
from syntax_highlighter import AsmHighlighter
from settings import AppSettings
from settings_dialog import SettingsDialog


def _find_core_build_dir():
    """Locate the directory containing the compiled rv32i_core extension.

    Single-config generators (Unix Makefiles/Ninja, the default on
    Linux/macOS) put it directly in core/build/. Multi-config generators
    (Visual Studio, CMake's default on Windows) put it in a
    per-configuration subdirectory like core/build/Release/ instead.
    """
    base = os.path.abspath(os.path.join(os.path.dirname(__file__), "../core/build"))
    candidates = [base] + [
        os.path.join(base, cfg)
        for cfg in ("Release", "RelWithDebInfo", "Debug", "MinSizeRel")
    ]
    for candidate in candidates:
        if glob.glob(os.path.join(candidate, "rv32i_core*")):
            return candidate
    return base  # fall back so the import below raises a clear error


# Add the path to the compiled pybind11 module
module_path = _find_core_build_dir()
if module_path not in sys.path:
    sys.path.insert(0, module_path)

import rv32i_core


def _app_version():
    """Read the app version from the repo-root VERSION file (single source
    of truth, also used by RELEASE_PROCESS.md when cutting a release)."""
    version_path = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", "VERSION"))
    try:
        with open(version_path) as f:
            return f.read().strip()
    except OSError:
        return "unknown"


def _vendored_toolchain_dir():
    """The platform-specific vendor/toolchain/<platform>/bin/ produced by
    scripts/fetch_toolchain.py -- matches that script's platform-key logic."""
    if sys.platform == "win32":
        key = "win32-x64"
    elif sys.platform == "darwin":
        key = "darwin-arm64" if platform.machine().lower() in ("arm64", "aarch64") else "darwin-x64"
    else:
        key = "linux-x64"
    return os.path.abspath(
        os.path.join(os.path.dirname(__file__), "..", "vendor", "toolchain", key, "bin")
    )


def _find_assembler_tools():
    """Locate the RISC-V `as`/`objcopy` binaries the Assemble button needs.

    Prefers a vendored toolchain (what packaged releases bundle) over
    whatever's on PATH, and tries both xPack's naming
    (riscv-none-elf-*, what scripts/fetch_toolchain.py vendors) and the
    traditional riscv-gnu-toolchain naming (riscv64-unknown-elf-*, what
    this project originally documented) since a developer could have
    either installed. Returns (as_path, objcopy_path), or (None, None) if
    neither is found anywhere.
    """
    exe_suffix = ".exe" if sys.platform == "win32" else ""
    vendor_bin = _vendored_toolchain_dir()
    vendored_as = os.path.join(vendor_bin, f"riscv-none-elf-as{exe_suffix}")
    vendored_objcopy = os.path.join(vendor_bin, f"riscv-none-elf-objcopy{exe_suffix}")
    if os.path.exists(vendored_as) and os.path.exists(vendored_objcopy):
        return vendored_as, vendored_objcopy

    for as_name, objcopy_name in (
        ("riscv-none-elf-as", "riscv-none-elf-objcopy"),
        ("riscv64-unknown-elf-as", "riscv64-unknown-elf-objcopy"),
    ):
        as_path = shutil.which(as_name)
        objcopy_path = shutil.which(objcopy_name)
        if as_path and objcopy_path:
            return as_path, objcopy_path

    return None, None

from PyQt5.QtCore import QThread, pyqtSignal

class RunWorker(QThread):
    # Emits the new register snapshot after each step
    step_done = pyqtSignal(list, int, int, int)
    finished = pyqtSignal()
    breakpoint_hit = pyqtSignal(int)
    error = pyqtSignal(str)

    # How many instructions to execute before yielding to the GUI.
    # Larger = faster execution, smaller = more responsive UI updates.
    BATCH_SIZE = 100

    def __init__(self, cpu, max_steps=10_000_000, breakpoints=None, parent=None):
        super().__init__(parent)
        self.cpu = cpu
        self._is_running = True
        # Safety cap so an infinite loop (j halt) doesn't spin forever.
        # Set to 0 to disable the cap.
        self.max_steps = max_steps
        self.breakpoints = breakpoints or set()

    def stop(self):
        self._is_running = False

    def _snapshot(self, steps):
        regs = list(self.cpu.get_registers())
        pc = self.cpu.get_pc()
        try:
            inst_bytes = self.cpu.read_memory(pc, 4)
            inst = int.from_bytes(inst_bytes, byteorder='little')
        except Exception:
            inst = 0
        self.step_done.emit(regs, pc, inst, steps)

    def run(self):
        steps = 0
        hit_breakpoint = False
        while self._is_running:
            if self.max_steps and steps >= self.max_steps:
                break
            try:
                success = self.cpu.step()
                steps += 1
                if not success:
                    break
                if self.cpu.get_pc() in self.breakpoints:
                    hit_breakpoint = True
                    break
                if steps % self.BATCH_SIZE == 0:
                    self._snapshot(steps)
            except Exception as e:
                self.error.emit(str(e))
                break

        # Final snapshot
        self._snapshot(steps)
        if hit_breakpoint:
            self.breakpoint_hit.emit(self.cpu.get_pc())
        self.finished.emit()

UI_FILE = os.path.join(os.path.dirname(__file__), "./ui/main_window.ui")
APP_ICON_PATH = os.path.join(os.path.dirname(__file__), "../resources/icons/app.png")
APP_NAME = "rv32i-emulator"

# A single dark theme tying the editor, docks, and chrome together so the
# app reads as one cohesive tool instead of mismatched default widgets.
DARK_STYLE = """
QMainWindow, QDialog { background-color: #252526; color: #D4D4D4; }
QMenuBar { background-color: #2D2D30; color: #D4D4D4; }
QMenuBar::item:selected { background-color: #3E3E42; }
QMenu { background-color: #2D2D30; color: #D4D4D4; border: 1px solid #3E3E42; }
QMenu::item:selected { background-color: #094771; }
QToolBar { background-color: #2D2D30; border: none; spacing: 4px; padding: 2px; }
QStatusBar { background-color: #007ACC; color: #FFFFFF; }
QStatusBar QLabel { color: #FFFFFF; }
QDockWidget { color: #D4D4D4; titlebar-close-icon: none; }
QDockWidget::title { background-color: #2D2D30; padding: 4px; }
QPlainTextEdit, QTextEdit, QLineEdit, QSpinBox {
    background-color: #1E1E1E; color: #D4D4D4;
    border: 1px solid #3E3E42; selection-background-color: #264F78;
}
QTableWidget, QListWidget {
    background-color: #252526; color: #D4D4D4;
    gridline-color: #3E3E42; border: 1px solid #3E3E42;
    alternate-background-color: #2A2D2E;
}
QHeaderView::section { background-color: #2D2D30; color: #D4D4D4; border: none; padding: 4px; }
QPushButton {
    background-color: #3E3E42; color: #D4D4D4; border: 1px solid #555;
    padding: 4px 10px; border-radius: 3px;
}
QPushButton:hover { background-color: #4E4E52; }
QCheckBox, QLabel { color: #D4D4D4; }
QTabWidget::pane { border: 1px solid #3E3E42; background-color: #252526; }
QTabBar::tab {
    background-color: #2D2D30; color: #D4D4D4;
    border: 1px solid #3E3E42; border-bottom: none;
    padding: 4px 10px;
}
QTabBar::tab:selected { background-color: #252526; border-bottom: 2px solid #007ACC; }
QTabBar::tab:hover:!selected { background-color: #3E3E42; }
"""

class MainWindow(QMainWindow):
    def __init__(self):
        super().__init__()
        uic.loadUi(UI_FILE, self)
        self.setStyleSheet(DARK_STYLE)
        self._upgrade_editor()

        if os.path.exists(APP_ICON_PATH):
            self.setWindowIcon(QIcon(APP_ICON_PATH))

        self.current_file = None
        self.cpu = rv32i_core.CPU()
        self.running = False
        self.worker = None
        self.cpu.reset()
        self.program_loaded = False
        self.breakpoints = set()
        self.pc_history = deque(maxlen=50)
        self.instructions_executed = 0
        self.app_settings = AppSettings()

        self.editor.setPlainText("Hello World!")
        self.editor.document().setModified(False)
        self.editor.document().modificationChanged.connect(lambda _: self._update_window_title())
        self._update_window_title()

        saved_font_size = self.app_settings.editor_font_size()
        font = self.editor.font()
        font.setPointSize(saved_font_size)
        self.editor.setFont(font)

        # A persistent "?" toolbar button that drops the cursor into Qt's
        # What's-This mode -- click it, then click any dock/control to see
        # an explanation of what it does (whatsThis text is set per-widget
        # below and in the .ui file's action definitions).
        self.actionWhatsThis = QWhatsThis.createAction(self)
        self.toolBar.addSeparator()
        self.toolBar.addAction(self.actionWhatsThis)

        # actions
        self.actionNew.triggered.connect(self.new_file)
        self.actionOpen.triggered.connect(self.open_file)
        self.actionRecent_Files.triggered.connect(self.show_recent_files_menu)
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
        self.actionStep_Over.triggered.connect(self.step_over)
        self.actionStep_Out.triggered.connect(self.step_out)
        self.actionPause.triggered.connect(self.pause)

        # Every dock gets an explicit objectName (required for
        # QMainWindow.saveState()/restoreState() to reliably round-trip a
        # layout) and is recorded in self.docks under a stable short key, so
        # later code (settings persistence, "show all" helpers) can iterate
        # by name instead of hard-coding each dock attribute individually.
        self.docks = {}

        # //register window dock
        self.register_dock = QDockWidget("Registers", self)
        self.register_dock.setObjectName("dock_registers")
        self.register_widget = RegisterWidget(self)
        registers_help = ("Lists all 32 CPU registers (x0-x31) with their ABI names and "
                          "current values. Registers changed by the last step are highlighted.")
        self.register_dock.setWhatsThis(registers_help)
        self.register_widget.setWhatsThis(registers_help)
        self.register_dock.setWidget(self.register_widget)
        self.addDockWidget(Qt.RightDockWidgetArea, self.register_dock)
        self.register_dock.setVisible(False)                # hidden by default
        self.docks["registers"] = self.register_dock

        # Forward status messages from register widget to main status bar
        self.register_widget.statusMessage.connect(self.statusbar.showMessage)

        # When the internal Close button is clicked, hide the dock
        self.register_widget.closeRequested.connect(self.register_dock.hide)

        # Synchronize menu action with dock visibility
        self.actionShow_Registers.toggled.connect(self.on_show_registers_toggled)
        self.register_dock.visibilityChanged.connect(self.actionShow_Registers.setChecked)

        # //memory window dock
        self.memory_dock = QDockWidget("Memory", self)
        self.memory_dock.setObjectName("dock_memory")
        self.memory_widget = MemoryWidget(self)
        memory_help = ("Shows a hex dump of emulator memory. Enter an address and click Go "
                      "to jump there, or check Follow PC to track the program counter.")
        self.memory_dock.setWhatsThis(memory_help)
        self.memory_widget.setWhatsThis(memory_help)
        self.memory_dock.setWidget(self.memory_widget)
        self.addDockWidget(Qt.RightDockWidgetArea, self.memory_dock)
        self.memory_dock.setVisible(False)
        self.docks["memory"] = self.memory_dock

        self.actionShow_Memory.toggled.connect(self.memory_dock.setVisible)
        self.memory_dock.visibilityChanged.connect(self.actionShow_Memory.setChecked)
        # Connect the action

        # //disassembly window dock
        self.disasm_dock = QDockWidget("Disassembly", self)
        self.disasm_dock.setObjectName("dock_disassembly")
        self.disasm_widget = DisassemblyWidget(self)
        disasm_help = ("Shows disassembled instructions around the current PC (highlighted). "
                       "Double-click a row to set or clear a breakpoint there.")
        self.disasm_dock.setWhatsThis(disasm_help)
        self.disasm_widget.setWhatsThis(disasm_help)
        self.disasm_dock.setWidget(self.disasm_widget)
        self.addDockWidget(Qt.RightDockWidgetArea, self.disasm_dock)
        self.disasm_dock.setVisible(False)
        self.docks["disassembly"] = self.disasm_dock

        self.disasm_widget.breakpointsChanged.connect(self.on_breakpoints_changed)
        self.actionShow_Disassembly.toggled.connect(self.disasm_dock.setVisible)
        self.disasm_dock.visibilityChanged.connect(self.actionShow_Disassembly.setChecked)

        # //stack window dock
        self.stack_dock = QDockWidget("Stack", self)
        self.stack_dock.setObjectName("dock_stack")
        self.stack_widget = StackWidget(self)
        stack_help = ("Shows memory near the current stack pointer (sp) -- useful for "
                     "inspecting function-call frames and local variables.")
        self.stack_dock.setWhatsThis(stack_help)
        self.stack_widget.setWhatsThis(stack_help)
        self.stack_dock.setWidget(self.stack_widget)
        self.addDockWidget(Qt.RightDockWidgetArea, self.stack_dock)
        self.stack_dock.setVisible(False)
        self.docks["stack"] = self.stack_dock

        self.actionShow_Stack.toggled.connect(self.stack_dock.setVisible)
        self.stack_dock.visibilityChanged.connect(self.actionShow_Stack.setChecked)

        # //PC history dock
        self.pc_history_dock = QDockWidget("PC History", self)
        self.pc_history_dock.setObjectName("dock_pc_history")
        self.pc_history_list = QListWidget(self)
        pc_history_help = ("Lists the last executed program-counter values. Double-click an "
                           "entry to jump to it in the Memory and Disassembly docks.")
        self.pc_history_dock.setWhatsThis(pc_history_help)
        self.pc_history_list.setWhatsThis(pc_history_help)
        self.pc_history_list.itemDoubleClicked.connect(self.on_pc_history_item_double_clicked)
        self.pc_history_dock.setWidget(self.pc_history_list)
        self.addDockWidget(Qt.RightDockWidgetArea, self.pc_history_dock)
        self.pc_history_dock.setVisible(False)
        self.docks["pc_history"] = self.pc_history_dock

        self.actionShow_PC_History.toggled.connect(self.pc_history_dock.setVisible)
        self.pc_history_dock.visibilityChanged.connect(self.actionShow_PC_History.setChecked)

        # //output console dock
        self.output_dock = QDockWidget("Output", self)
        self.output_dock.setObjectName("dock_output")
        self.output_console = QPlainTextEdit(self)
        output_help = "Logs assemble/run/breakpoint/error messages as they happen."
        self.output_dock.setWhatsThis(output_help)
        self.output_console.setWhatsThis(output_help)
        self.output_console.setReadOnly(True)
        self.output_console.setMaximumBlockCount(2000)
        self.output_console.setFont(QFont("Courier New", 10))
        self.output_dock.setWidget(self.output_console)
        self.addDockWidget(Qt.BottomDockWidgetArea, self.output_dock)
        self.output_dock.setVisible(False)
        self.docks["output"] = self.output_dock

        self.actionShow_Output.toggled.connect(self.output_dock.setVisible)
        self.output_dock.visibilityChanged.connect(self.actionShow_Output.setChecked)

        # //permanent status bar widgets - always visible, not just transient
        self.pc_status_label = QLabel("PC: --")
        self.instr_status_label = QLabel("Instr: 0")
        self.state_status_label = QLabel("Idle")
        for label in (self.pc_status_label, self.instr_status_label, self.state_status_label):
            label.setContentsMargins(6, 0, 6, 0)
            self.statusbar.addPermanentWidget(label)

        self._user_paused = False
        self._setup_default_layout()
        # Restore a previously saved window/dock layout if one exists; if
        # not (first run) or it fails to apply (e.g. after a Qt upgrade
        # changes the saveState() blob format), the default layout above
        # stands as-is.
        self.app_settings.restore_window_state(self)
        self._setup_icons()
        self._update_action_states()

    def _setup_default_layout(self):
        """Arrange the docks into something usable on first launch instead
        of an empty editor with every debugging view hidden."""
        self.resizeDocks([self.register_dock, self.disasm_dock],
                         [300, 300], Qt.Vertical)
        self.splitDockWidget(self.register_dock, self.disasm_dock, Qt.Vertical)
        self.tabifyDockWidget(self.memory_dock, self.stack_dock)
        self.tabifyDockWidget(self.memory_dock, self.pc_history_dock)
        self.memory_dock.raise_()

        # Show a sensible starter set; the rest stay one click away in View.
        self.actionShow_Registers.setChecked(True)
        self.actionShow_Disassembly.setChecked(True)
        self.actionShow_Memory.setChecked(True)

    def reset_to_default_layout(self):
        """Escape hatch for Settings > Layout > Reset, in case a restored
        or user-arranged layout gets into an unusable state."""
        for dock in self.docks.values():
            self.removeDockWidget(dock)
        for key, dock in self.docks.items():
            area = Qt.BottomDockWidgetArea if key == "output" else Qt.RightDockWidgetArea
            self.addDockWidget(area, dock)
        self._setup_default_layout()
        # _setup_default_layout() only calls setChecked(True) on the
        # show-actions, which is a no-op if an action was already checked
        # before this reset (Qt doesn't re-emit toggled for an unchanged
        # state) -- so drive visibility directly instead of depending on
        # that signal chain having something to react to.
        visible_by_default = {"registers", "disassembly", "memory"}
        for key, dock in self.docks.items():
            dock.setVisible(key in visible_by_default)
        self.statusbar.showMessage("Layout reset to defaults.", 2000)

    def _upgrade_editor(self):
        """Swap the plain QPlainTextEdit from the .ui file for a CodeEditor
        with line numbers + syntax highlighting, keeping the same object
        name/API so the rest of the code can keep using self.editor."""
        old_editor = self.editor
        layout = old_editor.parentWidget().layout()
        index = layout.indexOf(old_editor)

        new_editor = CodeEditor(self)
        new_editor.setObjectName("editor")
        new_editor.setFont(old_editor.font())

        layout.removeWidget(old_editor)
        old_editor.deleteLater()
        layout.insertWidget(index, new_editor)

        self.editor = new_editor
        self.highlighter = AsmHighlighter(self.editor.document())

    def _setup_icons(self):
        style = self.style()
        icon_map = {
            self.actionNew: QStyle.SP_FileIcon,
            self.actionOpen: QStyle.SP_DialogOpenButton,
            self.actionSave: QStyle.SP_DialogSaveButton,
            self.actionSave_as: QStyle.SP_DriveFDIcon,
            self.actionUndo: QStyle.SP_ArrowBack,
            self.actionRedo: QStyle.SP_ArrowForward,
            self.actionAssemble: QStyle.SP_DialogApplyButton,
            self.actionRun: QStyle.SP_MediaPlay,
            self.actionPause: QStyle.SP_MediaPause,
            self.actionStep: QStyle.SP_MediaSeekForward,
            self.actionStep_Over: QStyle.SP_MediaSkipForward,
            self.actionStep_Out: QStyle.SP_ArrowUp,
            self.actionSettings: QStyle.SP_FileDialogDetailedView,
            self.actionHelp_Contents: QStyle.SP_MessageBoxQuestion,
            self.actionAbout: QStyle.SP_MessageBoxInformation,
        }
        for action, std_icon in icon_map.items():
            action.setIcon(style.standardIcon(std_icon))

    def _update_window_title(self):
        """Title reflects what's actually loaded and whether it has unsaved
        changes, instead of always reading the static app name."""
        name = os.path.basename(self.current_file) if self.current_file else "Untitled"
        dirty = "*" if self.editor.document().isModified() else ""
        self.setWindowTitle(f"{dirty}{name} - {APP_NAME}")

    def _update_action_states(self):
        """Centralizes which execution actions make sense given the current
        program/running state, so stale buttons don't sit there enabled
        when clicking them would just show an error message."""
        loaded = self.program_loaded
        running = self.running
        self.actionRun.setEnabled(loaded)
        self.actionStep.setEnabled(loaded and not running)
        self.actionStep_Over.setEnabled(loaded and not running)
        self.actionStep_Out.setEnabled(loaded and not running)
        self.actionPause.setEnabled(loaded and running)

    def _reset_program_state(self):
        """Clear stale program/CPU state - call when the editor content is
        replaced (New/Open) so Run/Step don't act on a binary that no
        longer matches what's on screen."""
        self.program_loaded = False
        self.running = False
        self.instructions_executed = 0
        self.pc_history.clear()
        self.pc_history_list.clear()
        self.cpu.reset()
        self._update_action_states()
        self.state_status_label.setText("Idle")
        self.pc_status_label.setText("PC: --")
        self.instr_status_label.setText("Instr: 0")

    def new_file(self):
        self.statusbar.showMessage("New file triggered", 1000)
        self.editor.clear()
        self.editor.document().setModified(False)
        self.current_file = None
        self._update_window_title()
        self._reset_program_state()

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
            self.open_path(file_path)
        else:
            self.statusbar.showMessage("Open cancelled.", 2000)

    def open_path(self, file_path):
        """Load a file into the editor directly, without the Open dialog.
        Used by open_file() above and by main.py's --file command-line
        argument."""
        try:
            with open(file_path, "r") as f:
                content = f.read()
            self.editor.setPlainText(content)
            self.current_file = file_path
            self.statusbar.showMessage(f"Opened {file_path}", 3000)
            self.editor.document().setModified(False)
            self._update_window_title()
            self.app_settings.add_recent_file(file_path)
            self._reset_program_state()
        except Exception as e:
            self.statusbar.showMessage(f"Error opening file: {str(e)}", 5000)

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
                self._update_window_title()
                self.app_settings.add_recent_file(self.current_file)
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
                self.statusbar.showMessage(f"Saved as {file_path}", 3000)
                self.editor.document().setModified(False)
                self._update_window_title()
                self.app_settings.add_recent_file(file_path)
            except Exception as e:
                self.statusbar.showMessage(f"Error saving: {str(e)}", 5000)
        else:
            self.statusbar.showMessage("Save cancelled.", 2000)

    def exit_app(self):
        self.statusbar.showMessage("Exit triggered :(", 1000)
        self.close()

    def closeEvent(self, event):
        self.app_settings.save_window_state(self)
        super().closeEvent(event)

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

    def append_output(self, text):
        """Log a line to the Output console dock (visible via View > Show Output)."""
        self.output_console.appendPlainText(text)

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

    def update_register_display(self, reg_values=None, pc=None, inst=None, steps=None):
        """Update register dock, PC, and current instruction."""
        if not self.program_loaded:
            return

        if steps is not None:
            self.instructions_executed = steps

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
        if hasattr(self, 'disasm_widget'):
            self.disasm_widget.set_pc(pc)
        if hasattr(self, 'stack_widget'):
            self.stack_widget.update_follow_sp(reg_values[2])
        if hasattr(self, 'programCounterEdit'):
            self.programCounterEdit.setText(f"0x{pc:08X}")

        self.pc_status_label.setText(f"PC: 0x{pc:08X}")
        self.instr_status_label.setText(f"Instr: {self.instructions_executed}")

        if not self.pc_history or self.pc_history[-1] != pc:
            self.pc_history.append(pc)
            self.pc_history_list.addItem(f"0x{pc:08X}")
            self.pc_history_list.scrollToBottom()
            while self.pc_history_list.count() > self.pc_history.maxlen:
                self.pc_history_list.takeItem(0)

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

        # 4. Assemble using a vendored toolchain if packaged, else whatever's on PATH
        as_path, objcopy_path = _find_assembler_tools()
        if not as_path:
            msg = ("RISC-V toolchain not found. Expected a vendored copy under "
                  "vendor/toolchain/, or 'riscv-none-elf-as'/'objcopy' "
                  "(xPack) or 'riscv64-unknown-elf-as'/'objcopy' on PATH.")
            self.statusbar.showMessage(msg, 5000)
            self.append_output(msg)
            return

        try:
            # Step 4a: assemble to ELF (temporary file, because we only need binary)
            # We'll assemble directly to a temporary ELF and then objcopy to bin.
            import tempfile
            with tempfile.NamedTemporaryFile(suffix='.elf', delete=False) as tmp_elf:
                elf_path = tmp_elf.name

            # _zicsr: modern binutils split the CSR instructions (CSRRW/RS/RC/
            # WI/SI/CI) out of the base ISA string, so plain "rv32im" rejects
            # them with "extension `zicsr' required" even though the core
            # fully implements and tests them (core/tests/test_csrr*.cc).
            subprocess.run(
                [as_path, "-march=rv32im_zicsr", "-o", elf_path, self.current_file],
                check=True, capture_output=True, text=True
            )

            # Step 4b: convert ELF to binary at desired location
            subprocess.run(
                [objcopy_path, "-O", "binary", elf_path, bin_file],
                check=True, capture_output=True, text=True
            )

            # Clean up temporary ELF
            os.unlink(elf_path)

        except subprocess.CalledProcessError as e:
            self.statusbar.showMessage("Assembly failed - see Output panel.", 5000)
            self.append_output(f"Assembly failed:\n{e.stderr}")
            return
        except FileNotFoundError as e:
            msg = f"Toolchain invocation failed: {e}"
            self.statusbar.showMessage(msg, 5000)
            self.append_output(msg)
            return

        # 5. Load the binary into the CPU
        try:
            with open(bin_file, "rb") as f:
                program_bytes = f.read()
            self.cpu.reset()
            self.cpu.load_program(list(program_bytes))   # load at default TEXT_START
            self.program_loaded = True
            self.instructions_executed = 0
            self.pc_history.clear()
            self.pc_history_list.clear()
            # // giving cpu reference to the memory/disassembly/stack views
            self.memory_widget.set_cpu(self.cpu)
            self.disasm_widget.set_cpu(self.cpu)
            self.stack_widget.set_cpu(self.cpu)
            self._update_action_states()
            self.state_status_label.setText("Loaded")
            self.statusbar.showMessage(
                f"Assembled and loaded {os.path.basename(bin_file)} successfully.",
                3000
            )
            self.append_output(f"Assembled {os.path.basename(bin_file)} successfully.")
            self.update_register_display()   # reset registers to zero
        except Exception as e:
            self.statusbar.showMessage(f"Failed to load binary into CPU: {e}", 5000)
            self.append_output(f"Failed to load binary into CPU: {e}")

    def run(self):
        if not self.program_loaded:
            self.statusbar.showMessage("No program loaded. Assemble first.", 3000)
            return
        # If already running, clicking Run again stops it (toggle behaviour)
        if self.running:
            self.worker.stop()
            return

        self.running = True
        self._user_paused = False
        self.actionRun.setText("Stop")          # visual feedback
        self.state_status_label.setText("Running")
        self._update_action_states()
        # The RunWorker steps the CPU on a background thread; Go/Refresh on
        # Memory/Stack would call into the same cpu object from this thread
        # while that's happening, so disable them until the run stops.
        self.memory_widget.set_controls_enabled(False)
        self.stack_widget.set_controls_enabled(False)

        self.worker = RunWorker(self.cpu, max_steps=10_000_000,
                                breakpoints=set(self.breakpoints))
        # step_done now carries the register snapshot, so wire it directly
        self.worker.step_done.connect(self.update_register_display)
        self.worker.finished.connect(self.on_run_finished)
        self.worker.breakpoint_hit.connect(self.on_breakpoint_hit)
        self.worker.error.connect(self.on_run_error)
        self.worker.start()
        self.statusbar.showMessage("Running…", 0)   # 0 = stay until replaced
        self.append_output("Run started.")

    def pause(self):
        if self.running and self.worker is not None:
            self._user_paused = True
            self.worker.stop()
            self.statusbar.showMessage("Pausing…", 0)

    def on_run_finished(self):
        self.running = False
        self.actionRun.setText("Run")
        self._update_action_states()
        self.memory_widget.set_controls_enabled(True)
        self.stack_widget.set_controls_enabled(True)
        self.worker = None
        # Final register refresh (reads PC too)
        self.update_register_display()
        if self._user_paused:
            self.state_status_label.setText("Paused")
            self.statusbar.showMessage(
                f"Paused. Executed {self.instructions_executed} instructions so far.", 3000)
            self.append_output(f"Paused. Executed {self.instructions_executed} instructions so far.")
        else:
            self.state_status_label.setText("Halted")
            self.statusbar.showMessage(
                f"CPU halted. Executed {self.instructions_executed} instructions.", 3000)
            self.append_output(f"CPU halted. Executed {self.instructions_executed} instructions.")
        self._user_paused = False

    def on_run_error(self, msg):
        self.statusbar.showMessage(f"Run error: {msg}", 5000)
        self.append_output(f"Run error: {msg}")
        self.on_run_finished()

    def on_breakpoint_hit(self, pc):
        self.statusbar.showMessage(f"Hit breakpoint at 0x{pc:08X}", 5000)
        self.append_output(f"Hit breakpoint at 0x{pc:08X}")

    def on_breakpoints_changed(self, breakpoints):
        self.breakpoints = set(breakpoints)

    def on_pc_history_item_double_clicked(self, item):
        addr = int(item.text(), 16)
        self.disasm_widget.go_to_address(addr)
        self.memory_widget.go_to_address(addr)

    def step(self):
        if not self.program_loaded:
            self.statusbar.showMessage("No program loaded. Assemble first.", 3000)
            return
        if self.running:
            self.statusbar.showMessage("Stop the running program first.", 2000)
            return
        try:
            success = self.cpu.step()
            self.instructions_executed += 1
            self.update_register_display(steps=self.instructions_executed)
            if not success:
                self.statusbar.showMessage("CPU halted.", 3000)
            else:
                pc = self.cpu.get_pc()
                self.statusbar.showMessage(
                    f"Stepped → PC = 0x{pc:08X} "
                    f"({self.instructions_executed} instr executed)", 2000)
        except Exception as e:
            self.statusbar.showMessage(f"Step error: {e}", 5000)

    def _run_until(self, target_pc, max_steps=1_000_000):
        """Single-step in a tight loop until PC reaches target_pc, a
        breakpoint is hit, or the CPU halts. Used by Step Over/Step Out."""
        steps = 0
        halted = False
        while steps < max_steps:
            success = self.cpu.step()
            steps += 1
            if not success:
                halted = True
                break
            pc = self.cpu.get_pc()
            if pc == target_pc or pc in self.breakpoints:
                break

        self.instructions_executed += steps
        self.update_register_display(steps=self.instructions_executed)
        if halted:
            self.statusbar.showMessage("CPU halted.", 3000)
        else:
            pc = self.cpu.get_pc()
            self.statusbar.showMessage(
                f"Stepped → PC = 0x{pc:08X} ({steps} instr)", 3000)

    def step_over(self):
        if not self.program_loaded:
            self.statusbar.showMessage("No program loaded. Assemble first.", 3000)
            return
        if self.running:
            self.statusbar.showMessage("Stop the running program first.", 2000)
            return
        try:
            pc = self.cpu.get_pc()
            word_bytes = self.cpu.read_memory(pc, 4)
            word = int.from_bytes(word_bytes, byteorder='little')
            opcode = word & 0x7F
            rd = (word >> 7) & 0x1F
            is_call = opcode in (OPCODE_JAL, OPCODE_JALR) and rd == 1  # rd = ra
            if is_call:
                self._run_until(pc + 4)
            else:
                self.step()
        except Exception as e:
            self.statusbar.showMessage(f"Step Over error: {e}", 5000)

    def step_out(self):
        if not self.program_loaded:
            self.statusbar.showMessage("No program loaded. Assemble first.", 3000)
            return
        if self.running:
            self.statusbar.showMessage("Stop the running program first.", 2000)
            return
        try:
            return_addr = self.cpu.get_registers()[1]  # ra
            self._run_until(return_addr)
        except Exception as e:
            self.statusbar.showMessage(f"Step Out error: {e}", 5000)

    def show_simulator(self):
        """Open the full debugging layout in one click: registers,
        disassembly, memory, stack, and PC history."""
        for action in (self.actionShow_Registers, self.actionShow_Disassembly,
                       self.actionShow_Memory, self.actionShow_Stack,
                       self.actionShow_PC_History):
            action.setChecked(True)
        self.disasm_dock.raise_()
        self.statusbar.showMessage("Simulator views shown.", 2000)

    def settings(self):
        dialog = SettingsDialog(self, self)
        dialog.exec()

    def show_recent_files_menu(self):
        files = self.app_settings.recent_files()
        if not files:
            self.statusbar.showMessage("No recent files.", 2000)
            return

        menu = QMenu(self)
        for path in files:
            action = menu.addAction(path)
            action.triggered.connect(lambda checked=False, p=path: self.open_path(p))
        menu.addSeparator()
        clear_action = menu.addAction("Clear Recent Files")
        clear_action.triggered.connect(self.app_settings.clear_recent_files)
        menu.exec(QCursor.pos())

    def help_contents(self):
        shortcuts = (
            "<h3>Keyboard shortcuts</h3>"
            "<table>"
            "<tr><td>F9</td><td>Assemble</td></tr>"
            "<tr><td>F5</td><td>Run / Stop</td></tr>"
            "<tr><td>F10</td><td>Step Over</td></tr>"
            "<tr><td>Shift+F11</td><td>Step Out</td></tr>"
            "<tr><td>F8</td><td>Show all debug views</td></tr>"
            "<tr><td>Ctrl+S / Ctrl+Shift+S</td><td>Save / Save As</td></tr>"
            "<tr><td>Ctrl+F / Ctrl+H</td><td>Find / Replace</td></tr>"
            "</table>"
            "<p>Double-click a row in the Disassembly view to toggle a "
            "breakpoint. Use View to show/hide Registers, Memory, "
            "Disassembly, Stack, PC History, and Output.</p>"
        )
        QMessageBox.information(self, "Help", shortcuts)

    def about(self):
        QMessageBox.about(
            self, "About",
            "<h3>RISC-V Linux Emulator</h3>"
            f"<p>Version {_app_version()}</p>"
            "<p>A RISC-V (RV32I + RV32M) CPU emulator with a PyQt5 GUI: "
            "assemble, run, and step through programs with live register, "
            "memory, disassembly, and stack views.</p>"
        )
