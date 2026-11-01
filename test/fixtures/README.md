# Historical fixture handling

Firmware builds use the single sibling `../ConfigurationsManager` source checkout
declared in the shared PlatformIO `[env]`. No frozen CM archive is an active build
dependency. Exact historical test inputs and their hash manifests belong in
ignored repository-local `.Temp/`; they are not public test fixtures.
