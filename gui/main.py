import argparse
import sys

from PyQt5.QtCore import Qt
from PyQt5.QtWidgets import QApplication

# Must be set before the QApplication is constructed.
QApplication.setAttribute(Qt.AA_EnableHighDpiScaling, True)
QApplication.setAttribute(Qt.AA_UseHighDpiPixmaps, True)

from main_window import MainWindow


def parse_args(argv):
    parser = argparse.ArgumentParser(description="RV32I emulator GUI")
    parser.add_argument("--file", metavar="PATH", help="Open this file on launch")
    return parser.parse_args(argv)


def main():
    args = parse_args(sys.argv[1:])
    app = QApplication(sys.argv)
    main_window = MainWindow()
    if args.file:
        main_window.open_path(args.file)
    main_window.show()
    sys.exit(app.exec_())

if __name__ == "__main__":
    main()
