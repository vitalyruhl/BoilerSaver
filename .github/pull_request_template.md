## Summary

Describe the BoilerSaver behavior or maintenance change and its user impact.

## Validation

List relevant commands and actual results. For firmware/build changes, include
`pio run -e usb`; build `testesp` when station behavior is affected. For the build
wrapper, run `python -m unittest discover -s test -p "test_*.py"`.
Report device uploads and bounded runtime observations separately from builds.
Documentation-only changes need link/path and `git diff --check` validation.

## Compatibility and documentation

- [ ] Guides reflect the final configuration and behavior, or no update is needed.
- [ ] Firmware version/date in `src/settings.h` follow the version policy.
- [ ] Settings/NVS, GPIO, relay and OTA impacts are described where relevant.
- [ ] Shared CM source checkout and transport dependencies are identified.
- [ ] No credentials, private configuration, raw device logs or local agent files are included.

## Authorization

Identify any authorized device test. Build or test evidence does not authorize a
production deployment, commit, push, merge or release.
