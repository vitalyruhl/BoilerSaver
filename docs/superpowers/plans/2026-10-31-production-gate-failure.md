# BoilerSaver 4.1.0 Production Gate Failure and Coverage Analysis

> Historical record: the original follow-up proposal below was superseded by the [operator-mapped station test](2026-10-31-station-peripheral-test.md) and the [private CM 4.10.6 station test](2026-10-31-cm4106-station-test.md). Use their confirmed mapping and current test scope. Historical production-panic causality remains unproven.

## Deployment status

**The 4.1.0 / CM 4.10.5 candidate failed its production gate with ESP_RST_PANIC and is not approved for production. Production was returned to BoilerSaver 4.0.4.** Recovery observation lasted 306.219 seconds with strictly increasing uptime (30573 to 316725 ms), 95 fresh MQTT messages and no observed reset. Its raw strict gate result is false solely because two HTTP reads timed out; this is not a zero-error HTTP acceptance. All 47 settings were rechecked afterward. The recovery gate and field comparison are recorded under the ignored `.pio/evidence/` directory. No commit, push or release was performed.

## Verified evidence

- CM 4.10.5 is the published registry release and upstream tag/main `7b4c9873f2846a246f97113b3fe67f6b5d4c235e`.
- USB, OTA and mocked COM8 builds passed; the original COM8 gate passed for 300.032 seconds without an unexpected reset.
- The real 4.1.0 device failed a 303.265-second HTTP/MQTT gate. Uptime fell from 206667 to 7459 ms, then from 185509 to 7289 ms. MQTT System-Info reported `resetReason: 4`, which the installed IDF header defines as `ESP_RST_PANIC`.
- A total of 129 fresh MQTT messages were observed during that failed cohort. Sampled free heap ranged from 142240 to 154892 bytes. This does not establish ordinary heap exhaustion; heap corruption or another exception still requires a backtrace.
- No production panic stack/backtrace was available through the observed MQTT output. The reset classification does not identify the faulting function.
- Relay state was OFF, shower request was false and timer was zero in every successful production sample. No production relay, shower or button test command was sent.
- Two thresholds returned to source defaults after the initial update: 55/80 became 60/78. Only those two original values were restored with individual `/config/save` calls. One explicitly authorized controlled reboot verified persistent 55/80 and relay OFF. The cause of the original drift remains unproved.
- All 47 pre-existing exposed settings matched after recovery. Passwords in the config export are masked; this is not a full NVS or credential backup.
- Two ArduinoOTA attempts failed before completion; the diagnostic attempt recorded Windows `ConnectionAbortedError(10053)`. HTTP OTA succeeded for the update and for recovery. A failed HTTP recovery attempt was verified not to have changed the running version before the successful retry.
- Exact failed BINs/ELFs are preserved in `.pio/evidence/failed-4.1.0/`, with hashes in `failed-4.1.0-artifacts.json`. The failed production gate is `production-4.1.0-failed-gate.json`.
- Recovery OTA image: 1196624 bytes, SHA256 `e8b204ac7efcc5f722e7733170f3efc9cbbde72c57f187fe44f2f6203c948229`. Esptool verified its checksum and embedded validation hash. It matches the previous version and observed sketch size; bit identity with the original flash was not established.

## What the passing COM8 bench did not exercise

| Boundary | Passing bench behavior | Missing production coverage |
| --- | --- | --- |
| IOManager registration, begin and update | Registration returns early; begin/update are skipped | Real persisted IO bindings, GPIO reconfiguration/polling, button events and IO-generated runtime sources |
| Relay state | A logical boolean replaces IOManager get/set | Physical GPIO23 readback and concurrent IO/runtime access |
| Temperature acquisition | Fixed 70 C with explicit [MOCKED!] provenance; no sensor/ticker setup | OneWire/DallasTemperature, real GPIO26, requestTemperatures conversion waits and periodic sensor callback |
| Display | Initialization, display writes and display wake/off callbacks return early | Wire/I2C, SSD1306 buffer transfer, main-loop display writes and timer-driven commands |
| LED helper | PulseOutput instance and loop are omitted | Physical status pulse helper and its interaction with other tasks |
| Hardware | COM8 ESP32-D0WD-V3 rev3.1, USB only | Production ESP32-D0WDQ6 rev1, actual sensor/display wiring and power conditions |

The production display on-time was 360 seconds. A short test that never reaches that deadline cannot exercise its first off callback. A USB-only test without an OLED or DS18B20 also cannot reproduce successful I2C transactions or the real conversion wait, even when their functions are re-enabled.

## Runtime task-boundary assessment

The source does **not** support the claim that demand-driven Live newly moved all getters from the main loop to an asynchronous context. The local CM 4.2.2 baseline already calls `runtimeJsonProvider()` inside the AsyncWebServer `/runtime.json` handler (`src/web/WebServer.cpp:1085`). Its runtime implementation explicitly describes concurrent WS-push and HTTP contexts (`src/runtime/RuntimeManager.cpp:165`). CM 4.10.5 adds selected HTTP generation and selected WebSocket projection; HTTP remains an AsyncTCP callback, while normal WebSocket snapshots are produced from the ConfigManager loop/push path.

CM 4.10.5 selects/copies registry entries under `runtimeDataMutex`, then executes user callbacks outside that registry mutex (`src/runtime/RuntimeManager.cpp:100`). Runtime generation has its own serialization guard. This protects CM bookkeeping; it does not synchronize consumer-owned variables with the sensor timer, main loop or MQTT callbacks.

For the Boiler card specifically:

- `temperature` is a `float`; `boilerTimeRemaining` is an `int`, not a shared String.
- The formatted time getter snapshots the integer into a local value, formats a local char buffer and returns a new String by value. No concurrently replaced global String was found in that getter.
- Other Boiler getters read scalar settings/state. The can-shower getter also writes a shared boolean.
- GPIO reads and IO/alarm-generated providers remain relevant cross-context boundaries. Alarm registration is performed during startup, but alarm state changes in the loop.
- The sensor callback runs through Arduino Ticker and requests blocking temperature conversion. The display timer can issue Wire/SSD1306 commands while the main loop also updates the display. These paths were omitted by the passing bench.

These are investigation boundaries, not validated causes. The available evidence identifies a panic and a coverage gap; it does not justify a race fix, a CM patch or a production trial.

## Hardware-assumption correction

The operator subsequently clarified that COM8 is a populated fixture with I2C, OneWire and other attached hardware. Earlier USB-only, unloaded and absent-peripheral descriptions were assumptions and must not be treated as verified facts. A full-path diagnostic had been started with the unverified production-derived pin assignment; active HTTP/browser load was stopped immediately after the correction. No further upload, reset, GPIO or control command was issued. Passive serial/MQTT preservation continues while the actual mapping is established.

The interrupted cohort reached a 300.05-second checkpoint with one intentional boot, no observed panic/reset, sensor callback/completion counts29/29 and I2C ACK status0. It is not the requested660-second acceptance and does not validate the wiring. Zero DS18B20 devices on configuredGPIO26 does not prove the fixture has no sensor on another pin. I2C ACK does not establish device identity. Logical relayfalse means configuredGPIO23 HIGH and does not establish the physical load state.

The effective unverified FullIO assignments were: relay23(LOW-active), shower19/pull-up, reset14/pull-up, AP13/pull-up, sensor26, I2CSDA21/SCL22, OLEDreset4 and statusLED2. FullIO binaries, ELF and matching source/config snapshots are preserved under `.pio/evidence/full-io-artifacts/`; interruption evidence is `full-io-interrupted.json`. The follow-up proposal below is suspended until actual pin/device/load mapping is verified.

## Concrete next test proposal

Keep production on recovered 4.0.4. Prepare a separate COM8 diagnostic build with unchanged CM 4.10.5 and all production IO/sensor/display call paths enabled, while retaining forced DHCP, distinct test client/topic namespace and explicit missing-sensor provenance. COM8 must remain USB-only with no heater/load or real MQTT actuator peer.

Capture serial output continuously, including the full panic/backtrace and exact matching ELF. First run the ordinary isolated baseline without a browser, then the same selected Boiler/System read-only Live navigation used previously. Observe for at least 660 seconds per relevant cohort so the persisted 360-second display timeout and a further 300-second interval are included. Do not send shower/button/relay commands.

If it panics, decode the backtrace with that exact ELF and only then isolate one proven path at a time. If it does not panic, the result narrows the failure to the still-missing real peripheral/power/chip-revision conditions; it does not qualify 4.1.0 for production. Matching isolated OLED and DS18B20 hardware would then be required for equivalent coverage.

No speculative CM or consumer concurrency fix was implemented, and no additional 4.1.0 production test is authorized by this report.
