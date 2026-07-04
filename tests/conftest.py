"""Shared pytest fixtures/setup for both the pycpu-binding tests and the GUI
smoke tests. Puts the compiled `rv32i_core` module and the `gui/` package on
sys.path so tests can import them the same way gui/main_window.py already
does, without every test file repeating the same path juggling.
"""
import glob
import os
import sys

import pytest

ROOT_DIR = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
GUI_DIR = os.path.join(ROOT_DIR, "gui")


def _find_core_build_dir():
    """Same resolution gui/main_window.py uses: single-config generators
    (Linux/macOS) put the compiled module directly in core/build/;
    multi-config generators (Visual Studio, CMake's Windows default) put it
    in a per-configuration subdirectory like core/build/Release/ instead.
    """
    base = os.path.join(ROOT_DIR, "core", "build")
    candidates = [base] + [
        os.path.join(base, cfg)
        for cfg in ("Release", "RelWithDebInfo", "Debug", "MinSizeRel")
    ]
    for candidate in candidates:
        if glob.glob(os.path.join(candidate, "rv32i_core*")):
            return candidate
    return base


for path in (_find_core_build_dir(), GUI_DIR):
    if path not in sys.path:
        sys.path.insert(0, path)

# Headless by default so `pytest` works in CI / over SSH without a display.
# Override by exporting QT_QPA_PLATFORM=xcb (or similar) before running pytest
# if you want to watch the GUI tests execute.
os.environ.setdefault("QT_QPA_PLATFORM", "offscreen")


@pytest.fixture(autouse=True)
def isolate_qsettings(tmp_path):
    """MainWindow.__init__ constructs a gui.settings.AppSettings, which
    reads/writes real QSettings storage (~/.config/rv32i-emulator/... on
    Linux) -- without this, every test that builds a MainWindow would touch
    the developer's actual settings file. Redirect ini-format QSettings to
    a fresh per-test temp directory instead."""
    from PyQt5.QtCore import QSettings
    QSettings.setPath(QSettings.IniFormat, QSettings.UserScope, str(tmp_path))
    yield
