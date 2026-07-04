#!/usr/bin/env bash
# One-command bootstrap for Linux/macOS: create the venv if needed, install
# dependencies, configure + build the C++ core, and run both test suites.
#
# Usage: ./scripts/setup.sh [--no-venv]
#   --no-venv   skip venv creation/activation, use whatever `python3` is
#               already on PATH (useful in CI where the environment is
#               already isolated).
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$ROOT_DIR"

USE_VENV=1
for arg in "$@"; do
  case "$arg" in
    --no-venv) USE_VENV=0 ;;
    *) echo "Unknown argument: $arg" >&2; exit 1 ;;
  esac
done

if [[ "$USE_VENV" -eq 1 ]]; then
  if [[ ! -d venv ]]; then
    echo "==> Creating venv/"
    python3 -m venv venv
  fi
  # shellcheck disable=SC1091
  source venv/bin/activate
fi

echo "==> Installing Python dependencies"
python3 -m pip install --quiet --upgrade pip
python3 -m pip install --quiet -r requirements.txt -r requirements-dev.txt

echo "==> Configuring CMake (core/build)"
cmake -S core -B core/build -DCMAKE_BUILD_TYPE=Release

echo "==> Building"
cmake --build core/build -j

echo "==> Running C++ tests (ctest)"
ctest --test-dir core/build --output-on-failure

echo "==> Running Python tests (pytest)"
python3 -m pytest

echo "==> Done. Launch the GUI with: python gui/main.py"
