#! /bin/bash
# python-appimage's `build app` command templates this file (substituting
# {{ python-executable }}) and wraps it as the AppImage's AppRun entry point.
# gui/, core/build/, and vendor/toolchain/ are placed directly under
# $APPDIR by scripts/build_appimage.sh's staging step, matching this
# project's normal relative layout so gui/main_window.py's existing
# ../core/build and vendor/toolchain path-resolution logic works unmodified.
exec {{ python-executable }} "${APPDIR}/gui/main.py" "$@"
