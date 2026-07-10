# Versioning

## App version

This project follows [Semantic Versioning](https://semver.org/):
`MAJOR.MINOR.PATCH`, tracked in a single `VERSION` file at the repo root and
shown in the GUI's About dialog.

- **PATCH** — bug fixes, no new features, no behavior changes.
- **MINOR** — new features, backwards compatible (e.g. a new instruction, a
  new GUI dock, a new packaging target).
- **MAJOR** — breaking changes to the CLI, saved settings schema, or public
  `rv32i_core` Python API (`2.0.0`). `0.x.y` is pre-1.0/unstable — anything
  can change between minor versions; `1.0.0` is the first stable release,
  after which the above rules are enforced strictly.

Pre-release tags (`-beta.N`, `-rc.N`) are used ahead of `1.0.0` and, if
needed, ahead of later major versions:

- `v0.1.0-beta.1`, `v0.1.0-beta.2`, ... — early public builds, features and
  packaging targets still being filled in (see `future_plans.md` for what's
  still missing from a given beta).
- `v0.1.0-rc.1` — feature-complete for `1.0.0`, fixes only.
- `v1.0.0` — first stable release.

`.github/workflows/release.yml`'s `prerelease` flag is derived directly from
the tag name (`-beta`/`-rc` substrings mark it as a pre-release on GitHub) —
keep tag names consistent with this scheme or the release will be flagged
incorrectly.

## Not the same as the app version

- **Target ISA** (RV32I + RV32M) — versioned by the RISC-V spec itself, not
  by this project.
- **Vendored toolchain version** (`XPACK_VERSION` in
  `scripts/fetch_toolchain.py`) — the xPack `riscv-none-elf-gcc` release used
  to source the bundled `as`/`objcopy` binaries, updated independently of the
  app version.
- **Bundled Python version** in the Linux AppImage — determined at build
  time by whatever `python3` compiled `core/` (see `scripts/build_appimage.sh`),
  not pinned to the app version.
