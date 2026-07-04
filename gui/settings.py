"""Thin QSettings wrapper for everything the GUI needs to remember across
restarts: window/dock layout, editor font size, and recently opened files.
Ini format is used (not the platform-native registry/plist backend) so the
settings file is the same kind of thing on every OS and easy to inspect/
delete by hand if something goes wrong.
"""
from PyQt5.QtCore import QSettings

ORG_NAME = "rv32i-emulator"
APP_NAME = "rv32i-emulator"

DEFAULT_FONT_SIZE = 10
MAX_RECENT_FILES = 10


class AppSettings:
    def __init__(self):
        self._qs = QSettings(QSettings.IniFormat, QSettings.UserScope, ORG_NAME, APP_NAME)

    def save_window_state(self, window):
        self._qs.setValue("window/geometry", window.saveGeometry())
        self._qs.setValue("window/state", window.saveState())

    def restore_window_state(self, window):
        """Returns True if a saved layout was found and applied. Callers
        should keep whatever default layout they already built if this
        returns False (first run, or a corrupted/incompatible save)."""
        geometry = self._qs.value("window/geometry")
        state = self._qs.value("window/state")
        if geometry is None or state is None:
            return False
        try:
            ok_geometry = window.restoreGeometry(geometry)
            ok_state = window.restoreState(state)
            return bool(ok_geometry and ok_state)
        except Exception:
            # QMainWindow.saveState()'s blob is tied to Qt's version and the
            # exact set of dock object names -- if either changed since this
            # was saved, fall back to the caller's default layout instead of
            # crashing on a stale/incompatible save.
            return False

    def editor_font_size(self):
        return int(self._qs.value("editor/font_size", DEFAULT_FONT_SIZE))

    def set_editor_font_size(self, size):
        self._qs.setValue("editor/font_size", int(size))

    def recent_files(self):
        files = self._qs.value("recent_files", [])
        if isinstance(files, str):  # QSettings collapses a 1-item list to a str
            files = [files]
        return list(files or [])

    def add_recent_file(self, path):
        files = self.recent_files()
        if path in files:
            files.remove(path)
        files.insert(0, path)
        files = files[:MAX_RECENT_FILES]
        self._qs.setValue("recent_files", files)
        return files

    def clear_recent_files(self):
        self._qs.setValue("recent_files", [])
