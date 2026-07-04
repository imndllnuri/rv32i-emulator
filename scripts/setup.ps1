# One-command bootstrap for Windows (PowerShell): create the venv if needed,
# install dependencies, configure + build the C++ core, and run both test
# suites. Mirrors scripts/setup.sh -- see that file for the Linux/macOS path.
#
# Usage: .\scripts\setup.ps1 [-NoVenv]
param(
    [switch]$NoVenv
)

$ErrorActionPreference = "Stop"
$RootDir = Split-Path -Parent $PSScriptRoot
Set-Location $RootDir

if (-not $NoVenv) {
    if (-not (Test-Path "venv")) {
        Write-Host "==> Creating venv/"
        python -m venv venv
    }
    & .\venv\Scripts\Activate.ps1
}

Write-Host "==> Installing Python dependencies"
python -m pip install --quiet --upgrade pip
python -m pip install --quiet -r requirements.txt -r requirements-dev.txt

Write-Host "==> Configuring CMake (core/build)"
cmake -S core -B core/build -DCMAKE_BUILD_TYPE=Release

Write-Host "==> Building"
cmake --build core/build --config Release -j

Write-Host "==> Running C++ tests (ctest)"
ctest --test-dir core/build --output-on-failure -C Release

Write-Host "==> Running Python tests (pytest)"
python -m pytest

Write-Host "==> Done. Launch the GUI with: python gui/main.py"
