# Original-workspace recovery

The owner requested recovery of relevant temporary ESP32 work into its original
repositories, publication of needed work branches, and in-place branch selection.
The active BoilerSaver directory is `C:\Daten\_Codding\ESP32\BoilerSaver` on
`feature/cm-4-10-5-light-live`; the former temporary checkout is not a development
workspace.

The previously pushed checkpoint is `59223a540d5c5d09f1610210f9b43f34f5f24537`.
The exact CM test package formerly selected from `C:\Temp` is now tracked under
`test/fixtures/`. Its SHA-256 remains
`b3cfc50834d81853d9ebe09afb7aca3d5f70db73e24726e9b58400cc4604a592`.
The `bench_cm_fix` dependency now uses that repository-relative archive.

This build-maintenance change requires the repository-policy patch bump from
4.1.0 to 4.1.1, dated 01.11.2026. Historical reports and frozen firmware hashes
remain records of 4.1.0. USB and OTA still pin released CM 4.10.5; bench-only
CM remains the exact private 4.10.6 fixture. No heating, IO, storage, settings,
transport implementation, or deployment behavior is changed by this recovery.

Private raw evidence, failed/recovery BIN and ELF files, and production-setting
snapshots are preserved locally under the original repository's ignored
`.pio/evidence/`, with copy hashes checked. They are deliberately not published.
Existing local editor/Serena changes are preserved and excluded from this
checkpoint. There is no existing project changelog; the maintenance change is
recorded here and in the README instead of inventing a release document.

The original production panic and historical strict HTTP failure remain open.
No production deployment, hardware test, main integration, or release is part
of this recovery. Validation of the new build is separate from historical
device observations.

## Verification

- `pio run -e usb -e bench_cm_fix`: both environments passed in the original
  repository. USB uses 58,684 bytes static RAM and 1,241,985 bytes flash;
  `bench_cm_fix` uses 58,620 bytes static RAM and 1,255,989 bytes flash.
- The installed bench package reports CM 4.10.6 and records
  `file://test/fixtures/cm-4.10.6-test-b3cfc50834d8.tar.gz` as its source.
- `python -m unittest discover -s test -p 'test_*.py'`: two wrapper tests passed.
- `git diff --check` passed. The tracked archive SHA-256 matches the frozen input.

These are offline build and host-test results; no upload or hardware observation
was performed for 4.1.1. The OTA environment was not rebuilt because its
dependency declarations and upload configuration were not changed.
