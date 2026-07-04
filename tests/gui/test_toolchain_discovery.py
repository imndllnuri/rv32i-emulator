"""Regression tests for gui/main_window.py's _find_assembler_tools(), which
prefers a vendored RISC-V toolchain (what packaged releases bundle via
scripts/fetch_toolchain.py) over whatever's on PATH.
"""
import os
import shutil

import pytest

main_window = pytest.importorskip("main_window", reason="PyQt5/gui import failed")


@pytest.fixture
def fake_vendor_toolchain(tmp_path, monkeypatch):
    """Creates a fake vendored toolchain and points _vendored_toolchain_dir()
    at it, without touching the real vendor/ directory."""
    vendor_dir = tmp_path / "vendor" / "toolchain" / "linux-x64" / "bin"
    vendor_dir.mkdir(parents=True)
    as_path = vendor_dir / "riscv-none-elf-as"
    objcopy_path = vendor_dir / "riscv-none-elf-objcopy"
    as_path.write_text("#!/bin/sh\necho fake-as\n")
    objcopy_path.write_text("#!/bin/sh\necho fake-objcopy\n")
    os.chmod(as_path, 0o755)
    os.chmod(objcopy_path, 0o755)

    monkeypatch.setattr(main_window, "_vendored_toolchain_dir", lambda: str(vendor_dir))
    return as_path, objcopy_path


def test_prefers_vendored_toolchain_when_present(fake_vendor_toolchain):
    as_path, objcopy_path = fake_vendor_toolchain

    found_as, found_objcopy = main_window._find_assembler_tools()

    assert found_as == str(as_path)
    assert found_objcopy == str(objcopy_path)


def test_falls_back_to_path_when_not_vendored(tmp_path, monkeypatch):
    # Point at an empty directory so no vendored copy is found.
    monkeypatch.setattr(main_window, "_vendored_toolchain_dir", lambda: str(tmp_path))

    real_as = shutil.which("riscv64-unknown-elf-as") or shutil.which("riscv-none-elf-as")
    if not real_as:
        pytest.skip("no RISC-V toolchain on PATH in this environment")

    found_as, found_objcopy = main_window._find_assembler_tools()

    assert found_as is not None
    assert "vendor" not in found_as


def test_returns_none_when_nothing_is_found(tmp_path, monkeypatch):
    monkeypatch.setattr(main_window, "_vendored_toolchain_dir", lambda: str(tmp_path))
    monkeypatch.setattr(shutil, "which", lambda name: None)

    found_as, found_objcopy = main_window._find_assembler_tools()

    assert found_as is None
    assert found_objcopy is None
