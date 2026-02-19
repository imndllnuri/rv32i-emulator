import os
from .find_dialog import FindDialog
from .replace_dialog import ReplaceDialog
from PyQt5 import uic
from PyQt5.QtWidgets import QFileDialog, QMainWindow, QStatusBar, QDialog
from PyQt5.QtGui import QFont

UI_FILE = os.path.join(os.path.dirname(__file__), './ui/main_window.ui')

class MainWindow(QMainWindow):
    def __init__(self):
        super().__init__()
        uic.loadUi(UI_FILE, self)

        self.current_file = None


        self.editor.setPlainText("Hello World!")
        #actions
        self.actionNew.triggered.connect(self.new_file)
        self.actionOpen.triggered.connect(self.open_file)
        self.actionRecent_Files.triggered.connect(lambda: self.statusbar.showMessage("Recent files triggered", 1000))  # or open recent menu
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
        self.actionShow_Registers.toggled.connect(self.toggle_registers)
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

        self.actionToggle_Comment.triggered.connect(self.toggle_comment)
        self.actionUntoggle_Comment.triggered.connect(self.untoggle_comment)

        self.actionStep_Into.triggered.connect(self.step_into)
        self.actionStep_Onto.triggered.connect(self.step_onto)
        self.actionStep_Out.triggered.connect(self.step_out)

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
                with open(file_path, 'r') as f:
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
                with open(self.current_file, 'w') as f:
                    f.write(self.editor.toPlainText())
                self.statusbar.showMessage(f"Saved {self.current_file}", 3000)
                # we have just saved it so there is not any modification yet (clean state)
                self.editor.document().setModified(False)
            except Exception as e:
                self.statusbar.showMessage(f"Error saving: {str(e)}", 5000)
        else:
            #if self.current_file = false we should behave as save_file_as
            self.save_file_as()


    def save_file_as(self):
        self.statusbar.showMessage("Save as triggered", 1000)

        # Use a dialog that enforces a default suffix
        dialog = QFileDialog(self)
        dialog.setWindowTitle("Save File As")
        dialog.setNameFilter("Assembly Files (*.x68);;Assembly Files (*.asm);;Text Files (*.txt);;All Files (*)")
        dialog.setDefaultSuffix("x68")   # appends .x68 if no extension typed
        dialog.setAcceptMode(QFileDialog.AcceptSave)

        if dialog.exec():
            file_path = dialog.selectedFiles()[0]
            self.current_file = file_path
            try:
                with open(file_path, 'w') as f:
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

    def toggle_registers(self, checked):
        self.statusbar.showMessage(f"Registers window toggled: {checked}", 1000)
        # TODO: show/hide registers dock

    def toggle_memory(self, checked):
        self.statusbar.showMessage(f"Memory view toggled: {checked}", 1000)
        # TODO: show/hide memory dock

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

    def assemble(self):
        self.statusbar.showMessage("Assemble triggered", 1000)
        # TODO: call external assembler

    def run(self):
        self.statusbar.showMessage("Run triggered", 1000)
        # TODO: start simulator

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

    def toggle_comment(self):
        self.statusbar.showMessage("Toggle comment triggered", 1000)
        # TODO: comment/uncomment selected lines

    def untoggle_comment(self):
        self.statusbar.showMessage("Untoggle comment triggered", 1000)
        # TODO: uncomment selected lines

    def step_into(self):
        self.statusbar.showMessage("Step Into triggered", 1000)
        # TODO: simulator step into

    def step_onto(self):
        self.statusbar.showMessage("Step Onto triggered", 1000)
        # TODO: simulator step over

    def step_out(self):
        self.statusbar.showMessage("Step Out triggered", 1000)
        # TODO: simulator step out
