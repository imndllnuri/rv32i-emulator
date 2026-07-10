# Release Process

1. Ensure `main` is green: `./scripts/setup.sh` (or `setup.ps1` on Windows)
   exits 0 — build succeeds, `ctest` and `pytest` both pass.
2. Move the `[Unreleased]` section of `CHANGELOG.md` into a new dated version
   section, following [Keep a Changelog](https://keepachangelog.com/en/1.1.0/).
3. Bump the version in `VERSION` per [VERSIONING.md](VERSIONING.md).
4. Commit as `chore: release vX.Y.Z` on a feature branch, open a PR, wait for
   CI to pass on all 3 OS legs, merge to `main`.
5. Tag the merge commit on `main` and push the tag:
   ```sh
   git tag vX.Y.Z
   git push origin vX.Y.Z
   ```
   This triggers `.github/workflows/release.yml`, which re-verifies the full
   test suite on Linux/Windows/macOS, builds the Linux AppImage, and opens a
   **draft** GitHub Release with it attached.
6. Once the workflow finishes, download the AppImage from the draft release
   and smoke-test it in a stripped environment (no venv, no toolchain on
   `PATH`) to confirm it's genuinely self-contained:
   ```sh
   chmod +x rv32i-emulator-x86_64.AppImage
   env -i ./rv32i-emulator-x86_64.AppImage
   ```
   Assemble and run a trivial `.s` program end to end.
7. The workflow's auto-generated release notes are minimal (just a compare
   link). Replace them with the actual `CHANGELOG.md` section for that
   version:
   ```sh
   gh release edit vX.Y.Z --notes-file <(sed -n '/## \[X.Y.Z\]/,/## \[/p' CHANGELOG.md | sed '$d')
   ```
   (adjust the `sed` range to the actual section, or just paste it by hand —
   whichever is less error-prone in the moment).
8. Manually review the draft (notes + attached AppImage) on GitHub, then
   publish it. **Do not publish without a final look** — this is a public,
   hard-to-fully-retract action once people start downloading it.

## Cadence

No fixed cadence yet at this stage (pre-1.0, single maintainer) — cut a new
beta when there's a meaningful chunk of new functionality or packaging
coverage (e.g. Windows/macOS support landing), not on a calendar. Revisit
once past `v1.0.0`.
