import sys
from PyQt5.QtWidgets import QApplications
from src.gui.main_window import MainWindow

def main():
    app = QApplications(sys.argv)
    main_window = MainWindow()
    main_window.show()
    sys.exit(app.exec_())

if __name__ = "__main__"
    main()
