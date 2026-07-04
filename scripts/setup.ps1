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

# $ErrorActionPreference only governs PowerShell cmdlet errors, not the exit
# code of a native/external command (python, cmake, ctest, ...). Without an
# explicit check here, a failed step (e.g. a bad pip install) leaves
# PowerShell to plow ahead into cmake/build/ctest/pytest, each failing for a
# different downstream reason and burying the real error under noise.
function Invoke-Checked {
    param([string]$Description)
    if ($LASTEXITCODE -ne 0) {
        Write-Error "FAILED: $Description (exit code $LASTEXITCODE)"
        exit $LASTEXITCODE
    }
}

if (-not $NoVenv) {
    if (-not (Test-Path "venv")) {
        Write-Host "==> Creating venv/"
        python -m venv venv
        Invoke-Checked "python -m venv venv"
    }
    & .\venv\Scripts\Activate.ps1
}

Write-Host "==> Installing Python dependencies"
python -m pip install --quiet --upgrade pip
Invoke-Checked "pip install --upgrade pip"
python -m pip install --quiet -r requirements.txt -r requirements-dev.txt
Invoke-Checked "pip install -r requirements.txt -r requirements-dev.txt"

Write-Host "==> Configuring CMake (core/build)"
cmake -S core -B core/build -DCMAKE_BUILD_TYPE=Release
Invoke-Checked "cmake configure"

Write-Host "==> Building"
cmake --build core/build --config Release -j
Invoke-Checked "cmake --build"

Write-Host "==> Running C++ tests (ctest)"
ctest --test-dir core/build --output-on-failure -C Release
Invoke-Checked "ctest"

Write-Host "==> Running Python tests (pytest)"
python -m pytest
Invoke-Checked "pytest"

Write-Host "==> Done. Launch the GUI with: python gui/main.py"
