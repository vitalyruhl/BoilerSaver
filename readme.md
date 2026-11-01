# BoilerSaver (ESP32)

This example is a larger real-world project based on the ConfigurationsManager library.
It combines runtime controls, sensor input, optional MQTT integration, and Web UI configuration.

## What it demonstrates

- Complex settings structure with grouped pages/cards
- Runtime actions and live values (`/runtime.json`, `/runtime_meta.json`)
- Optional MQTT defaults via `src/secret/secrets.h`
- Integration of external components (e.g. OneWire/DallasTemperature, OLED)

## How to run

From the original repo root, with the sibling `../ConfigurationsManager` checkout
available. All environments use its current source branch through the shared
`[env]` dependency declaration. Keep the ESP32Async transport pair aligned with
that checkout.

```bash
pio run -e usb
pio run -e testesp
python -m unittest discover -s test -p "test_*.py"
```

## First start / AP mode

If no SSID is configured yet, the device starts in AP mode.
Open the printed AP URL from Serial (usually `http://192.168.4.1`) and configure WiFi via the Web UI.

## Optional secrets defaults

You can copy `src/secret/secrets.example.h` to `src/secret/secrets.h` and set default WiFi/MQTT credentials.

## Additional docs

- [Wiring and operational mapping](docs/BoilerSaver-esp32-connection.md)
- [Home Assistant entities](docs/HomeAssistant.yaml)
- [Home Assistant dashboard](docs/HomeAssistant_Dashboard.yaml)

## Build profiles and test station

Common options and dependencies are declared once in `[env]`. `usb` selects the
normal USB port, `ota` selects the configured network target, and `testesp`
selects COM8 with the operator-confirmed station wiring. Uploading is a separate
operator action; a successful build does not establish production readiness.

`testesp` uses the same application and CM source with DS18B20 GPIO18, I2C
SDA21/SCL22, OLED reset -1, unloaded dummy boiler output LED25, status LED26 and
inputs14/33/4. Relay pins19/27/23 stay unregistered. Startup applies DHCP, the
isolated `BoilerSaver-Test-COM8` MQTT client/topic namespace, disabled heating and
a -1000 C lower threshold in RAM after loading NVS; it does not save these
overrides during startup.

```bash
pio run -e testesp -t upload --upload-port COM8
```

Verify COM8 identity and wiring before an authorized upload. Use normal
application upload and preserve NVS. Report sensor/display/IO checks and bounded
runtime observations separately from production approval and long-term stability.
Never publish private settings or raw device logs.
