#!/usr/bin/env bash
# Builds a self-contained Linux AppImage using python-appimage (bundles a
# relocatable CPython + PyQt5, so end users need nothing preinstalled).
#
# Prerequisites: core/ already built (cmake --build core/build), and
# `pip install python-appimage` in the active environment.
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$ROOT_DIR"

# Must match the Python version core/build's rv32i_core*.so was compiled
# against -- pybind11 extensions are ABI-tied to an exact CPython minor
# version, so bundling a mismatched interpreter here fails at import time
# with a confusing "ModuleNotFoundError" rather than a clear ABI error.
RECIPE_DIR="resources/packaging/appimage"
PYTHON_VERSION="$(python3 -c 'import sys; print(f"{sys.version_info.major}.{sys.version_info.minor}")')"

if [[ ! -d core/build ]] || ! ls core/build/rv32i_core*.so >/dev/null 2>&1; then
  echo "core/ hasn't been built yet. Run ./scripts/setup.sh first." >&2
  exit 1
fi

if [[ ! -f vendor/toolchain/linux-x64/bin/riscv-none-elf-as ]]; then
  echo "==> Vendoring the RISC-V toolchain"
  python3 scripts/fetch_toolchain.py --platform linux-x64
fi

echo "==> Staging payload (gui/, core/build/, vendor/toolchain/, VERSION)"
STAGE_DIR="$(mktemp -d)"
trap 'rm -rf "$STAGE_DIR"' EXIT

mkdir -p "$STAGE_DIR/gui" "$STAGE_DIR/core/build" "$STAGE_DIR/vendor/toolchain"
cp -r gui/. "$STAGE_DIR/gui/"
cp -r core/build/. "$STAGE_DIR/core/build/"
cp -r vendor/toolchain/. "$STAGE_DIR/vendor/toolchain/"
# Sibling of gui/ so _app_version()'s os.path.dirname(__file__)/../VERSION
# resolves the same way inside the AppImage as it does from a normal checkout.
cp VERSION "$STAGE_DIR/VERSION"

echo "==> Syncing recipe metadata"
cp requirements.txt "$RECIPE_DIR/requirements.txt"
cp resources/icons/app.png "$RECIPE_DIR/rv32i-emulator.png"

echo "==> Building the AppImage"
# appdir must come before -x: -x's nargs='+' greedily consumes everything
# after it on the command line, including a positional arg placed after it.
python3 -m python_appimage build app \
  "$RECIPE_DIR" \
  -p "$PYTHON_VERSION" \
  -n rv32i-emulator \
  -x "$STAGE_DIR/gui" "$STAGE_DIR/core" "$STAGE_DIR/vendor" "$STAGE_DIR/VERSION"

mkdir -p dist
mv rv32i-emulator-*.AppImage dist/
echo "==> Done: $(ls dist/rv32i-emulator-*.AppImage)"
