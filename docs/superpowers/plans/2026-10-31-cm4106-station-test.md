# BoilerSaver 4.1.0 private CM 4.10.6 station test

The bounded runtime/peripheral gate **passed**. The strict HTTP gate **failed** because of one timeout. The historical 4.1.0 production panic remains unresolved as a cause. Production was last restored to 4.0.4 and was not accessed during this test.

## Frozen inputs and isolation

Environment `bench_cm_fix` uses private CM 4.10.6, ESPAsyncWebServer 3.12.1 and AsyncTCP 3.5.0. No esphome forks were installed in this environment. Other library versions and Arduino 2.0.17/IDF 4.4.7 remained unchanged. Installed CM header/runtime-message sources match the frozen archive byte for byte and appear in the actual compiler dependencies.

The reviewed BIN was written directly from the freeze at app offset `0x10000` without recompilation. A 4-KiB read confirmed the actual COM8 partition table matched the build; NVS was preserved. esptool reported `Hash of data verified.` Image size: 1,262,560 bytes; app slot: 1,310,720 bytes; reserve: 48,160 bytes.

Namespace/client/topic root: `BoilerSaver-Test-COM8-CM4106`; DHCP and empty static IP. Mapping: DS18 GPIO18, I2C SDA21/SCL22, OLED reset -1, dummy boiler output LED25 active HIGH, status LED26, buttons 14/33/4. Relay pins 19/27/23 remain reserved and unregistered. Test-only enabled=false and threshold -1000 prevent automatic heating demand. No settings, shower, relay or GPIO commands were issued. Main firmware logic is unchanged from the correctly mapped earlier station run.

Mapping authority: `C:\Daten\_Codding\ESP32\esp32-bacnet-stack\docs\hardware\esp32\IO-example-esp32-connection.md`.

## One continuous observation

Started UTC: `2026-10-31T18:24:16.286072+00:00`. Duration: 660.016 seconds. No repeat gate.

| Check | Observation |
| --- | --- |
| Panic / unexpected reset | None; one captured controlled baseline boot, no fault events |
| MQTT | 847 messages, including 131 runtime diagnostic snapshots |
| Uptime | Monotonic: diagnostics 5066→655725 ms, HTTP 3002→658671 ms; maximum diagnostic gap 5020 ms |
| Actual sensor | 1 device on GPIO18, offset 0; 129 valid snapshots at 24.9375–25.0000 °C; 2 explicitly unavailable startup snapshots |
| Sensor timer | Final 65 callback entries / 65 completions; intermediate snapshots had at most 1 callback in flight |
| OLED/I2C | ACK status 0, successful initialization, actual display-off callback 1× |
| Output | LED25 readback LOW in all 131 diagnostics; no sampled boiler-on or shower request |
| Boiler HTTP values | 104 successful responses, 468–474 bytes each |
| System HTTP values | 20 successful responses, 435–436 bytes each |
| WebSocket | Boiler/System Live views; two deliberate tab-close/new-document reconnects restored read-only Boiler values |
| HTTP limitation | One `TimeoutError: timed out` at 117.578 seconds; strict HTTP gate fails |
| WiFi | One startup-connected event; no serial disconnect or reconnect event |
| Heap | Sampled free 127304–156368 bytes; minimum-free watermark 114336 bytes; final sampled free 150220 bytes |

The monitor stopped normally and released COM8. The owned test tab was closed. COM8 retains the isolated firmware.

## Evidence and limits

- [Build provenance and hashes](../../../.pio/evidence/cm4106-build-manifest.json)
- [Raw result](../../../.pio/evidence/cm4106-station-result.json), [summary](../../../.pio/evidence/cm4106-station-summary.json)
- [Full serial](../../../.pio/evidence/cm4106-station-serial.log), [MQTT](../../../.pio/evidence/cm4106-station-mqtt.json), [UI observations](../../../.pio/evidence/cm4106-station-ui.json)
- [Evidence checksums](../../../.pio/evidence/cm4106-station-evidence.json), [frozen BIN/ELF/source inputs](../../../.pio/evidence/cm4106-build-artifacts/)

BIN SHA256: `b74dfe76ceaec63af71739870ffa2cded5589e013d8ecac20aafbb826e4d331d`.

ELF SHA256: `2cb9eaef6f2f085e0d702b1cc56f7202da99deaedf04b18b6aade6916f9d5617`.

CM package SHA256: `b3cfc50834d81853d9ebe09afb7aca3d5f70db73e24726e9b58400cc4604a592`.

No panic trace occurred. OLED evidence covers I2C acknowledgement, initialization return value and timer execution; physical pixels were not independently inspected. No heating-load or relay-contact acceptance was performed. WebSocket observations cover document recreation and restored Live values, without packet-level tracing or a reconnection-latency measurement.

The same mapped fixture previously completed 660 seconds with CM 4.10.5. Historical production-panic causality and long-term stability remain unproven. The isolated HTTP timeout has no established cause. Production 192.168.2.130 and COM7 were not accessed. No production deployment, commit, push or release was performed.

## End-of-session Git checkpoint

After the hardware observation, the user explicitly authorized committing and pushing the existing feature branch. This checkpoint covers the ten project and report files. The validated firmware sources and dependency configuration remain unchanged; existing build and 660-second observations are reused. No additional hardware test is performed for the checkpoint. Secrets, raw captures, BIN/ELF files and the private CM archive remain ignored and local; the private test environment requires that archive to rebuild. The earlier no-commit/no-push statement describes the hardware-test phase. Production deployment, merge and release remain outside this checkpoint.
