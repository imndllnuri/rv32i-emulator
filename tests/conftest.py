"""Shared pytest fixtures/setup for both the pycpu-binding tests and the GUI
smoke tests. Puts the compiled `rv32i_core` module and the `gui/` package on
sys.path so tests can import them the same way gui/main_window.py already
does, without every test file repeating the same path juggling.
"""
import os
import sys

ROOT_DIR = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
CORE_BUILD_DIR = os.path.join(ROOT_DIR, "core", "build")
GUI_DIR = os.path.join(ROOT_DIR, "gui")

for path in (CORE_BUILD_DIR, GUI_DIR):
    if path not in sys.path:
        sys.path.insert(0, path)

# Headless by default so `pytest` works in CI / over SSH without a display.
# Override by exporting QT_QPA_PLATFORM=xcb (or similar) before running pytest
# if you want to watch the GUI tests execute.
os.environ.setdefault("QT_QPA_PLATFORM", "offscreen")
