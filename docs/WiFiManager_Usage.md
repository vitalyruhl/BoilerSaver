# WiFiManager - Reusable WiFi Management for ESP32

Eine wiederverwendbare, non-blocking WiFi-Management-Klasse für ESP32-Projekte.

## Features

- ✅ **Non-blocking WiFi-Operationen** - blockiert nie die main loop
- ✅ **Automatische Reconnection** - intelligente Wiederverbindung bei Verbindungsabbruch
- ✅ **Auto-Reboot Funktionalität** - optional nach konfigurierbarem Timeout
- ✅ **State-Management** - klare Zustandsverfolgung
- ✅ **Callback-System** - Events für Connected/Disconnected/AP-Mode
- ✅ **AP-Mode Erkennung** - automatische Behandlung von Access Point Mode
- ✅ **Konfigurierbar** - Reconnect-Intervall und Timeouts anpassbar

## Verwendung

### 1. Include und Instanz erstellen

```cpp
#include "helpers/wifi_manager.h"

WiFiManager wifiManager; // Global instance
```

### 2. Callback-Funktionen definieren

```cpp
void onWiFiConnected() {
    Serial.println("WiFi connected! Starting services...");
    // Hier Services starten (MQTT, OTA, etc.)
}

void onWiFiDisconnected() {
    Serial.println("WiFi disconnected! Stopping services...");
    // Hier Services stoppen
}

void onWiFiAPMode() {
    Serial.println("WiFi in AP mode");
    // Spezielle AP-Mode Behandlung
}
```

### 3. Setup in setup() Funktion

```cpp
void setup() {
    // ... andere Setup-Code ...
    
    // WiFi Manager initialisieren
    wifiManager.begin(
        10000,  // Reconnect-Intervall in ms (10 Sekunden)
        30      // Auto-Reboot Timeout in Minuten (0 = deaktiviert)
    );
    
    // Callbacks registrieren
    wifiManager.setCallbacks(onWiFiConnected, onWiFiDisconnected, onWiFiAPMode);
}
```

### 4. Update in loop() Funktion

```cpp
void loop() {
    // WiFi Manager aktualisieren (non-blocking!)
    wifiManager.update();
    
    // ... rest der loop-Logik ...
}
```

## API-Referenz

### Initialisierung

```cpp
void begin(unsigned long reconnectIntervalMs = 10000, unsigned long autoRebootTimeoutMin = 0);
void setCallbacks(WiFiConnectedCallback onConnected, WiFiDisconnectedCallback onDisconnected, WiFiAPModeCallback onAPMode = nullptr);
```

### Status-Abfragen

```cpp
WiFiManagerState getState() const;          // Aktueller Zustand
bool isConnected() const;                   // WiFi verbunden?
bool isInAPMode() const;                    // Im AP-Modus?
unsigned long getLastConnectionTime() const; // Letzter Verbindungszeit
unsigned long getTimeSinceLastConnection() const; // Zeit seit letzter Verbindung
String getStatusString() const;             // Status als String
float getConnectionUptime() const;          // Verbindungszeit in Sekunden
```

### Konfiguration

```cpp
void enableAutoReboot(bool enable);                    // Auto-Reboot aktivieren/deaktivieren
void setAutoRebootTimeout(unsigned long timeoutMinutes); // Auto-Reboot Timeout setzen
void setReconnectInterval(unsigned long intervalMs);    // Reconnect-Intervall setzen
void forceReconnect();                                 // Sofortige Reconnection erzwingen
void reset();                                          // WiFi Manager zurücksetzen
```

## Zustände (WiFiManagerState)

- `WIFI_STATE_DISCONNECTED` - Nicht verbunden
- `WIFI_STATE_CONNECTING` - Verbindungsaufbau
- `WIFI_STATE_CONNECTED` - Erfolgreich verbunden
- `WIFI_STATE_AP_MODE` - Access Point Modus
- `WIFI_STATE_RECONNECTING` - Wiederverbindung wird versucht

## Beispiel für andere Projekte

```cpp
#include <WiFi.h>
#include "helpers/wifi_manager.h"

WiFiManager wifiManager;
bool servicesActive = false;

void onWiFiConnected() {
    Serial.println("✅ WiFi connected!");
    Serial.printf("IP: %s\n", WiFi.localIP().toString().c_str());
    
    // Deine Services hier starten
    startWebServer();
    startMQTT();
    servicesActive = true;
}

void onWiFiDisconnected() {
    Serial.println("❌ WiFi disconnected!");
    
    // Services stoppen
    if (servicesActive) {
        stopWebServer();
        stopMQTT();
        servicesActive = false;
    }
}

void setup() {
    Serial.begin(115200);
    
    // WiFi Credentials (über Configuration Manager oder direkt)
    WiFi.begin("YOUR_SSID", "YOUR_PASSWORD");
    
    // WiFi Manager konfigurieren
    wifiManager.begin(15000, 10); // 15s Reconnect, 10min Auto-Reboot
    wifiManager.setCallbacks(onWiFiConnected, onWiFiDisconnected);
}

void loop() {
    wifiManager.update(); // ⚠️ Wichtig: Immer in der loop() aufrufen!
    
    // Deine App-Logik hier...
    if (servicesActive) {
        handleWebServer();
        handleMQTT();
    }
    
    delay(10); // Kleine Pause für Watchdog
}
```

## Vorteile gegenüber manueller WiFi-Behandlung

1. **Keine Blocking-Operationen** - loop() läuft immer weiter
2. **Intelligente State-Machine** - saubere Zustandsverfolgung
3. **Wiederverwendbar** - einmal schreiben, überall nutzen
4. **Konfigurierbar** - an Projektbedürfnisse anpassbar
5. **Event-basiert** - saubere Trennung von WiFi-Logik und App-Logik
6. **Robustheit** - automatische Fehlerbehandlung und Recovery

## Integration in bestehende Projekte

1. Kopiere `wifi_manager.h` und `wifi_manager.cpp` in dein Projekt
2. Ersetze manuelle WiFi-Checks durch `wifiManager.update()`
3. Verschiebe Service-Start/Stop-Logik in die Callback-Funktionen
4. Entferne alle `delay()` Aufrufe aus WiFi-Reconnection-Code

Die WiFiManager-Klasse ist vollständig eigenständig und kann in jedem ESP32-Projekt verwendet werden!