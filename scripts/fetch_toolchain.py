#!/usr/bin/env python3
"""Vendor the RISC-V `as`/`objcopy` binaries the GUI's Assemble button needs,
so packaged releases work with zero manual setup. Downloads the prebuilt
xPack GNU RISC-V Embedded GCC distribution for the target platform and
extracts only the two binaries actually used at runtime -- not the full
toolchain (which is ~10x larger and otherwise unused, since assembling and
converting to a raw binary is all this project needs `as`/`objcopy` for).

Usage:
    python scripts/fetch_toolchain.py [--platform linux-x64|darwin-x64|darwin-arm64|win32-x64]

Output goes to vendor/toolchain/<platform>/bin/. Not committed to git --
packaging workflows run this script as a build step instead.
"""
import argparse
import hashlib
import io
import os
import platform
import shutil
import sys
import tarfile
import urllib.request
import zipfile

XPACK_VERSION = "15.2.0-1"
XPACK_TAG = f"v{XPACK_VERSION}"
XPACK_BASE_URL = (
    f"https://github.com/xpack-dev-tools/riscv-none-elf-gcc-xpack/releases/download/{XPACK_TAG}"
)

# platform key -> (archive suffix, archive format, exe suffix)
PLATFORMS = {
    "linux-x64": (f"xpack-riscv-none-elf-gcc-{XPACK_VERSION}-linux-x64.tar.gz", "tar.gz", ""),
    "darwin-x64": (f"xpack-riscv-none-elf-gcc-{XPACK_VERSION}-darwin-x64.tar.gz", "tar.gz", ""),
    "darwin-arm64": (f"xpack-riscv-none-elf-gcc-{XPACK_VERSION}-darwin-arm64.tar.gz", "tar.gz", ""),
    "win32-x64": (f"xpack-riscv-none-elf-gcc-{XPACK_VERSION}-win32-x64.zip", "zip", ".exe"),
}

BINARIES = ["riscv-none-elf-as", "riscv-none-elf-objcopy"]

ROOT_DIR = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
VENDOR_DIR = os.path.join(ROOT_DIR, "vendor", "toolchain")


def detect_platform():
    system = platform.system()
    machine = platform.machine().lower()
    if system == "Linux":
        return "linux-x64"
    if system == "Darwin":
        return "darwin-arm64" if machine in ("arm64", "aarch64") else "darwin-x64"
    if system == "Windows":
        return "win32-x64"
    raise SystemExit(f"Unsupported platform: {system} {machine}")


DOWNLOAD_TIMEOUT_SECONDS = 120


def download(url):
    print(f"Downloading {url} ...")
    with urllib.request.urlopen(url, timeout=DOWNLOAD_TIMEOUT_SECONDS) as response:
        return response.read()


def verify_checksum(data, sha_url):
    expected_line = download(sha_url).decode("utf-8").strip()
    expected_hash = expected_line.split()[0]
    actual_hash = hashlib.sha256(data).hexdigest()
    if actual_hash != expected_hash:
        raise SystemExit(
            f"Checksum mismatch: expected {expected_hash}, got {actual_hash}. "
            "Aborting -- refusing to extract an unverified archive."
        )
    print("Checksum OK.")


def extract_binaries(archive_bytes, archive_format, exe_suffix, out_dir):
    wanted = {f"bin/{name}{exe_suffix}" for name in BINARIES}
    found = set()
    os.makedirs(out_dir, exist_ok=True)

    if archive_format == "tar.gz":
        with tarfile.open(fileobj=io.BytesIO(archive_bytes), mode="r:gz") as tar:
            for member in tar:
                # Members look like "xpack-riscv-none-elf-gcc-.../bin/riscv-none-elf-as"
                rel = "/".join(member.name.split("/")[1:])
                if rel in wanted:
                    extracted = tar.extractfile(member)
                    out_path = os.path.join(out_dir, os.path.basename(rel))
                    with open(out_path, "wb") as f:
                        shutil.copyfileobj(extracted, f)
                    os.chmod(out_path, 0o755)
                    found.add(rel)
    elif archive_format == "zip":
        with zipfile.ZipFile(io.BytesIO(archive_bytes)) as zf:
            for name in zf.namelist():
                rel = "/".join(name.split("/")[1:])
                if rel in wanted:
                    out_path = os.path.join(out_dir, os.path.basename(rel))
                    with zf.open(name) as extracted, open(out_path, "wb") as f:
                        shutil.copyfileobj(extracted, f)
                    found.add(rel)
    else:
        raise SystemExit(f"Unknown archive format: {archive_format}")

    missing = wanted - found
    if missing:
        raise SystemExit(f"Archive did not contain expected binaries: {sorted(missing)}")


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--platform", choices=sorted(PLATFORMS), default=None,
                        help="Target platform (default: autodetect the current machine)")
    args = parser.parse_args()

    target = args.platform or detect_platform()
    archive_name, archive_format, exe_suffix = PLATFORMS[target]
    archive_url = f"{XPACK_BASE_URL}/{archive_name}"
    sha_url = f"{archive_url}.sha"
    out_dir = os.path.join(VENDOR_DIR, target, "bin")

    archive_bytes = download(archive_url)
    verify_checksum(archive_bytes, sha_url)
    extract_binaries(archive_bytes, archive_format, exe_suffix, out_dir)

    print(f"Vendored {', '.join(BINARIES)} for {target} -> {out_dir}")


if __name__ == "__main__":
    main()
