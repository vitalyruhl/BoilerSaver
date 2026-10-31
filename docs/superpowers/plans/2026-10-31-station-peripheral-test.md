# COM8 Station Peripheral Test

## Scope and status

This follow-up covers ordinary BoilerSaver test firmware and physical peripherals only. No CM/transport code was changed or repaired, no production request/write was made, and COM7 was not accessed. Production remains outside this test's deployment scope.

The runtime/peripheral observation completed for 660.093 seconds without panic or unexpected reset. The strict HTTP gate is false because three reads timed out; these results remain separate.

## Authoritative station mapping

Reference: C:/Daten/_Codding/ESP32/esp32-bacnet-stack/docs/hardware/esp32/IO-example-esp32-connection.md. The reference was read; no file in that project was modified.

| Test function | GPIO / configuration |
| --- | --- |
| DS18B20 | 18, 4.7 kOhm pull-up; zero test calibration offset |
| OLED/BME I2C | SDA21, SCL22, OLED0x3C |
| OLED reset | -1; four-pin module has no reset connection |
| Dummy boiler output | LED25, active-high, idleLOW |
| Status indication | LED26, active-high |
| Reset/Mid/Set inputs | 14/33/4, INPUT_PULLUP, low-active |
| Reserved relay pins | 19/27/23 are not registered or driven by the test profile |

The test profile forces DHCP and client/topic root BoilerSaver-Test-COM8-FullIO before networking. It disables heating and uses a test-only -1000 C lower threshold because the existing forced under-temperature path can otherwise request heat for a room-temperature probe. No shower, relay or button test commands are sent. Only LED25 is bound to the boiler output ID.

## Code and artifact evidence

- Environment: bench_full_io; source branch feature/cm-4-10-5-light-live.
- The production OLED reset4 and all17 checked production hardware/control/MQTT function bodies remain unchanged when test flags are disabled.
- Source/config and exact BIN/ELF snapshots: .pio/evidence/station-18-artifacts/.
- Manifest, hashes, effective bindings and physical-reference hash: .pio/evidence/station-18-artifacts.json.
- BIN SHA256: 47cf626e0cd32ce68817c61e65b80e652c83b83685c42a47840fbe5dd9a689ed.
- The test binary contains its explicit TEST identity and no production192.168.2.130 literal.
- The previous upload with incomplete station mapping had already finished when the complete mapping arrived. The board was then placed in the bootloader, no test workload was started for that image, and the corrected bindings were verified before the next upload.

## Initial actual hardware evidence

The corrected test found one DS18B20 onGPIO18. First valid raw/zero-offset temperatures were25.125 and25.0625C; pre-acquisition temperature fields were null. I2C returnedACK and the display initializer returnedtrue. Those facts establish bus response and initialization; they do not replace visual confirmation of rendered OLED pixels.

Runtime instrumentation counts sensor callback entry/completion and the display-off callback. The360-second display timer is retained, so the660-second observation covers that deadline. HTTP requests select only Boiler (including the actual IO source Boiler.shower_btn) or system. A separate owned Chrome tab supplies read-only WebSocket Live navigation.


## Completed result

- Runtime/peripheral gate passed: 660.093 seconds, one intentional startup reset, no observed panic or unexpected reset. No backtrace was produced.
- One DS18B20 was verified on GPIO18. Valid raw/zero-offset readings ranged from 24.9375 to 25.125 C; sensor callback/completion counts reached 65/65. A diagnostic snapshot may occur while one conversion callback is in flight; this is distinct from an unfinished final callback.
- Display initialization returned true and I2C probe status was 0 (ACK). The display-off callback executed once. WiFi disconnected at 235.767 s and reconnected at 246.997 s; the existing reconnect callback restarted the 360 s timer. The first observed off-callback count was at uptime 610656 ms.
- There were 122 selected HTTP samples, 125 diagnostic MQTT samples and 829 total observed test-namespace MQTT messages. Three HTTP requests timed out around reconnection; no extra cohort or restart hid them.
- Dummy output LED25 remained logically OFF throughout recorded diagnostics. Relay pins 19/27/23 were not registered. No shower, relay or button test command was sent.
- Read-only Chrome navigation exercised Boiler/IO and System pages in WebSocket mode. The owned tab was closed afterward.
- Exact source/config snapshots and BIN/ELF still match their manifest hashes. No CM/transport code, canonical production source or production setting was changed by this follow-up. No production upload is authorized or performed by this result.
- This establishes the exercised fixture paths, not the cause or remediation of the earlier production panic. It does not replace visual OLED pixel acceptance or longer-term stability evidence.

Machine-readable results: .pio/evidence/station-18-summary.json and station-18-result.json. Raw serial evidence: station-18-serial.log.
