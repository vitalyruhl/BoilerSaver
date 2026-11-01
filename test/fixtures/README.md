# Frozen CM test input

`cm-4.10.6-test-b3cfc50834d8.tar.gz` is the exact unpublished CM 4.10.6
package used for the 2026-10-31 COM8 station test. Its SHA-256 is:

```text
b3cfc50834d81853d9ebe09afb7aca3d5f70db73e24726e9b58400cc4604a592
```

The package is retained as a versioned test fixture because it contains the
captured generated web assets as well as source. A source checkout is not a
byte-identical substitute. It contains 312 regular files and no secrets
headers, local configuration, private directories, symlinks, or build caches.
CM's repository and license are included in the package.

Only `bench_cm_fix` selects this fixture. Run `pio run -e bench_cm_fix` from
the BoilerSaver repository root to compile it. USB and OTA retain their
released CM 4.10.5 dependency. This build command does not upload firmware.

The historical 660-second device observation applies to the original 4.1.0
image, not to a newly compiled 4.1.1 image. Its HTTP timeout and unproven
production-panic attribution remain open.
