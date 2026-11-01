# BoilerSaver (ESP32)

**Deployment status:** The 4.1.0 / CM 4.10.5 candidate failed the production gate with `ESP_RST_PANIC`. Production was returned to 4.0.4 with all 47 original settings preserved. Do not deploy the 4.1.0 candidate. See [the failure and coverage analysis](docs/superpowers/plans/2026-10-31-production-gate-failure.md).

The current work branch identifies as 4.1.1 after relocating its CM test package
into Git. This maintenance checkpoint does not resolve the historical panic or
authorize production deployment. See [the workspace recovery record](docs/superpowers/plans/2026-11-01-workspace-recovery.md).

This example is a larger real-world project based on the ConfigurationsManager library.
It combines runtime controls, sensor input, optional MQTT integration, and Web UI configuration.

## What it demonstrates

- Complex settings structure with grouped pages/cards
- Runtime actions and live values (`/runtime.json`, `/runtime_meta.json`)
- Optional MQTT defaults via `src/secret/secrets.h`
- Integration of external components (e.g. OneWire/DallasTemperature, OLED)

## How to run

From the repo root:

```bash
pio run -e usb
pio run -e usb -t upload
```

## First start / AP mode

If no SSID is configured yet, the device starts in AP mode.
Open the printed AP URL from Serial (usually `http://192.168.4.1`) and configure WiFi via the Web UI.

## Optional secrets defaults

You can copy `src/secret/secrets.example.h` to `src/secret/secrets.h` and set default WiFi/MQTT credentials.

## Additional docs

- Wiring and integration notes: `docs/`

## CM 4.10.5 / BoilerSaver 4.1.0

The firmware pins the released CM 4.10.5 package and keeps Live enabled. Existing
named providers and Live builders use demand-driven snapshots: the bundled UI
requests only the active page's groups. Settings and hidden views request the
small safety envelope. Legacy full `/runtime.json` consumers remain supported.
The build wrapper executes only the selected package's precompile hook.

Production heating, timer logic, IO defaults, settings keys, MQTT topic structure
and NVS remain unchanged. Display libraries retain the previously installed
versions to avoid an unrelated upgrade.

### Isolated COM8 bench

`pio run -e bench -t upload` targets COM8 only. This environment uses DHCP and
the distinct `BoilerSaver-Test-COM8` MQTT client/topic namespace even with stale
bench NVS. It skips IO initialization, GPIO polling/output, display operations
and sensor acquisition. Temperature is a fixed 70 C and relay state is simulated;
the app and diagnostic payloads identify this as `[MOCKED!]`. It must never be
uploaded to the production device. It validates firmware/Live delivery, not
physical heating or sensor accuracy.

Build the production candidate with `pio run -e usb` and `pio run -e ota`.
Production deployment remains a separate operator-reviewed action.

**COM8 hardware correction:** COM8 is a populated station. Its authoritative mapping is now applied only to `bench_full_io`: DS18GPIO18, I2CSDA21/SCL22, OLEDreset-1, dummy outputLED25, statusLED26 and inputs14/33/4. Relay pins19/27/23 remain unregistered. Test heating is disabled; a -1000C test lower threshold prevents automatic under-temperature requests. The earlier USB-only assumption and interrupted cohort are not accepted hardware evidence. See [the corrected peripheral test record](docs/superpowers/plans/2026-10-31-station-peripheral-test.md).

Private CM 4.10.6 COM8 integration testing is recorded in [the station test report](docs/superpowers/plans/2026-10-31-cm4106-station-test.md): the 660-second runtime/peripheral gate passed, with one separately failing HTTP timeout. Production remains on the recovered 4.0.4; the historical 4.1.0 production panic is not established as resolved.
