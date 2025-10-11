#include <Arduino.h>
#include <PubSubClient.h> // for MQTT
#include <esp_task_wdt.h> // for watchdog timer
#include <Ticker.h>
#include "Wire.h"

#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
AsyncWebServer server(80);

#include "settings.h"
#include "logging/logging.h"
#include "helpers/helpers.h"
#include "helpers/relays.h"

// predeclare the functions (prototypes)
void SetupStartDisplay();
void reconnectMQTT();
void handleMqttReconnection(); // Non-blocking MQTT reconnection handler
void cb_MQTT(char *topic, byte *message, unsigned int length);
void publishToMQTT();
void cb_PublishToMQTT();
void cb_MQTTListener();
void WriteToDisplay();
void SetupCheckForResetButton();
void SetupCheckForAPModeButton();
bool SetupStartWebServer();
void CheckButtons();
void ShowDisplay();
void ShowDisplayOff();
void updateStatusLED();
void PinSetup();

//--------------------------------------------------------------------------------------------------------------

#pragma region configuration variables

static const char GLOBAL_THEME_OVERRIDE[] PROGMEM = R"CSS(
h3 { color: orange; text-decoration: underline; }
)CSS";

Helpers helpers;

Ticker PublischMQTTTicker;
Ticker PublischMQTTTSettingsTicker;
Ticker ListenMQTTTicker;
Ticker displayTicker;

WiFiClient espClient;
PubSubClient client(espClient);

// globale helpers variables
float temperature = 0.0;      // current temperature in Celsius
int boilerTimeRemaining = 0; // remaining time for boiler in minutes
bool boilerState = false;    // current state of the heater (on/off)

bool tickerActive = false;    // flag to indicate if the ticker is active
bool displayActive = true;   // flag to indicate if the display is active

// WiFi downtime tracking for auto reboot
static unsigned long wifiLastGoodMillis = 0;     // last time WiFi was connected (and not AP)
static bool wifiAutoRebootArmed = false;         // becomes true after initial setup completion

// Non-blocking MQTT reconnection state management
enum MqttReconnectState {
  MQTT_STATE_IDLE,
  MQTT_STATE_CONNECTING,
  MQTT_STATE_WAIT_RESULT,
  MQTT_STATE_CONNECTED,
  MQTT_STATE_FAILED
};


//non blocking MQTT reconnection variables
static MqttReconnectState mqttState = MQTT_STATE_IDLE;
static unsigned long mqttLastAttempt = 0;
static int mqttRetryCount = 0;
static const int mqttMaxRetries = 10;
static const unsigned long mqttRetryInterval = 5000; // 5 seconds between attempts
static const unsigned long mqttConnectTimeout = 10000; // 10 seconds to wait for connection
// Non-blocking WiFi reconnection state management
static unsigned long lastWifiReconnectAttempt = 0;
static const unsigned long wifiReconnectInterval = 10000; // 10 seconds between attempts
// Non-blocking display update management
static unsigned long lastDisplayUpdate = 0;
static const unsigned long displayUpdateInterval = 100; // Update display every 100ms
#pragma endregion configuration variables

//----------------------------------------
// MAIN FUNCTIONS
//----------------------------------------

void setup()
{

  LoggerSetupSerial(); // Initialize the serial logger

  sl->Printf("System setup start...").Debug();

  cfg.setAppName(APP_NAME); // Set an application name, used for SSID in AP mode and as a prefix for the hostname
  cfg.setCustomCss(GLOBAL_THEME_OVERRIDE, sizeof(GLOBAL_THEME_OVERRIDE) - 1);// Register global CSS override
  cfg.enableBuiltinSystemProvider(); // enable the builtin system provider (uptime, freeHeap, rssi etc.)


  PinSetup();
  sl->Printf("Check for reset/AP button...").Debug();
  SetupCheckForResetButton();
  SetupCheckForAPModeButton();

  // sl->Printf("Clear Settings...").Debug();
  // cfg.clearAllFromPrefs();

  sl->Printf("Load configuration...").Debug();
  cfg.loadAll();

  cfg.checkSettingsForErrors();
  // Re-apply relay pin modes with loaded settings (pins/polarity may differ from defaults)
  Relays::initPins();

  mqttSettings.updateTopics();

  // init modules...
  sl->Printf("init modules...").Debug();
  SetupStartDisplay();
  ShowDisplay();

  helpers.blinkBuidInLEDsetpinMode(); // Initialize the built-in LED pin mode

  sl->Printf("Configuration printout:").Debug();
  Serial.println(cfg.toJSON(false)); // Print the configuration to the serial monitor
  //----------------------------------------

  bool isStartedAsAP = SetupStartWebServer();

  //----------------------------------------
  // -- Setup MQTT connection --
  sl->Printf("⚠️ SETUP: Starting MQTT! [%s]", mqttSettings.mqtt_server.get().c_str()).Debug();
  sll->Printf("Starting MQTT! [%s]", mqttSettings.mqtt_server.get().c_str()).Debug();
  client.setServer(mqttSettings.mqtt_server.get().c_str(), static_cast<uint16_t>(mqttSettings.mqtt_port.get())); // Set the MQTT server and port
  client.setCallback(cb_MQTT);

  sl->Debug("System setup completed.");
  sll->Debug("Setup completed.");

  // initialize WiFi tracking
  if(WiFi.status() == WL_CONNECTED && WiFi.getMode() != WIFI_AP){
    wifiLastGoodMillis = millis();
  } else {
    wifiLastGoodMillis = millis(); // start timer now; will reboot later if never connects
  }
  wifiAutoRebootArmed = true; // after setup we start watching

  //---------------------------------------------------------------------------------------------------
  // Runtime live values provider for relay outputs
  cfg.addRuntimeProvider({
    .name = String("Boiler"),
    .fill = [] (JsonObject &o){
        o["Bo_EN_Set"] = boilerSettings.enabled.get();
        o["Bo_EN"] = Relays::getBoiler();
        o["Bo_SettedTime"] = boilerSettings.boilerTimeMin.get();
        o["Bo_TimeLeft"] = boilerTimeRemaining;
        o["Bo_Temp"] = temperature;
    }
  });

  cfg.defineRuntimeField("Boiler", "Bo_Temp", "temperature", "°C", 1, 10);
  cfg.defineRuntimeField("Boiler", "Bo_TimeLeft", "time left", "min", 1, 60);


  // Add interactive controls Set-Boiler
  cfg.addRuntimeProvider({
      .name = "Hand overrides",
      .fill = [](JsonObject &o){ /* optionally expose current override states later */ }
  });

  static bool stateBtnState = false;
  cfg.defineRuntimeStateButton("Hand overrides", "sb_mode", "Will Duschen", [](){ return stateBtnState; }, [](bool v){
    stateBtnState = v;  Relays::setBoiler(v);}, /*init*/ false, 91);



  //---------------------------------------------------------------------------------------------------
}

void loop()
{

  
  CheckButtons();
  boilerState = Relays::getBoiler();

  if (WiFi.status() == WL_CONNECTED && WiFi.getMode() != WIFI_AP)
  {
    if (!tickerActive) // Check if the ticker is not already active
    {
      ShowDisplay(); // Show the display
      sl->Debug("WiFi connected! Reattach ticker.");
      sll->Debug("WiFi reconnected!");
      sll->Debug("Reattach ticker.");
      PublischMQTTTicker.attach(mqttSettings.MQTTPublischPeriod.get(), cb_PublishToMQTT); // Reattach the ticker if WiFi is connected
      ListenMQTTTicker.attach(mqttSettings.MQTTListenPeriod.get(), cb_MQTTListener);      // Reattach the ticker if WiFi is connected
      if(systemSettings.allowOTA.get()){
          sll->Debug("Start OTA-Module");
          cfg.setupOTA(APP_NAME, systemSettings.otaPassword.get().c_str());
      }
      tickerActive = true; // Set the flag to indicate that the ticker is active
    }
    // Update last good WiFi timestamp when connected (station mode only)
    wifiLastGoodMillis = millis();
  }
  else
  {
    if (tickerActive) // Check if the ticker is already active
    {
      ShowDisplay(); // Show the display to indicate WiFi is lost
      sl->Debug("WiFi not connected or in AP mode! deactivate ticker.");
      sll->Debug("WiFi lost connection!");
      sll->Debug("or run in AP mode!");
      sll->Debug("deactivate mqtt ticker.");
      PublischMQTTTicker.detach(); // Stop the ticker if WiFi is not connected or in AP mode
      ListenMQTTTicker.detach();   // Stop the ticker if WiFi is not connected or in AP mode
      tickerActive = false;        // Set the flag to indicate that the ticker is not active

      // check if ota is active and settings is off, reboot device, to stop ota
      if (systemSettings.allowOTA.get() == false && cfg.isOTAInitialized())
      {
        sll->Debug("Stop OTA-Module");
        cfg.stopOTA();
      }
    }
    
    // Non-blocking WiFi reconnection (only if not in AP mode)
    if (WiFi.getMode() != WIFI_AP) {
      unsigned long now = millis();
      if (now - lastWifiReconnectAttempt > wifiReconnectInterval) {
        lastWifiReconnectAttempt = now;
        sl->Debug("Attempting WiFi reconnection...");
        sll->Debug("reconnect to WiFi...");
        WiFi.reconnect(); // Non-blocking reconnect attempt
      }
    }
    
    // Auto reboot logic: only if not AP mode, feature enabled and timeout exceeded
    if (wifiAutoRebootArmed && WiFi.getMode() != WIFI_AP){
        int timeoutMin = systemSettings.wifiRebootTimeoutMin.get();
        if(timeoutMin > 0){
            unsigned long now = millis();
            unsigned long elapsedMs = now - wifiLastGoodMillis;
            unsigned long thresholdMs = (unsigned long)timeoutMin * 60000UL;
            if(elapsedMs > thresholdMs){
                sl->Printf("[WiFi] Lost for > %d min -> reboot", timeoutMin).Error();
                sll->Printf("WiFi lost -> reboot").Error();
                ESP.restart();
            }
        }
    }
  }

  // Non-blocking display updates
  if (millis() - lastDisplayUpdate > displayUpdateInterval) {
    lastDisplayUpdate = millis();
    WriteToDisplay();
  }

  if (WiFi.getMode() == WIFI_AP) {
    // Show we are in AP mode - non-blocking
    static unsigned long lastAPMessage = 0;
    if (millis() - lastAPMessage > 5000) {
      lastAPMessage = millis();
      sll->Debug("Running in AP mode!");
    }
  }

  // Evaluate cross-field runtime alarms periodically (cheap doc build ~ small JSON)
  static unsigned long lastAlarmEval = 0;
  if (millis() - lastAlarmEval > 1500)
  {
      lastAlarmEval = millis();
      cfg.handleRuntimeAlarms();
  }

  // Handle non-blocking MQTT reconnection
  handleMqttReconnection();

  updateStatusLED();
  cfg.handleClient();
  cfg.handleWebsocketPush();
  cfg.handleOTA();
  cfg.updateLoopTiming(); // Update internal loop timing metrics for system provider
}

//----------------------------------------
// MQTT FUNCTIONS
//----------------------------------------

void handleMqttReconnection()
{
  // Only handle MQTT when WiFi is connected and not in AP mode
  if (WiFi.status() != WL_CONNECTED || WiFi.getMode() == WIFI_AP) {
    mqttState = MQTT_STATE_IDLE;
    return;
  }

  unsigned long now = millis();

  switch (mqttState) {
    case MQTT_STATE_IDLE:
      // Check if MQTT is connected
      if (client.connected()) {
        mqttState = MQTT_STATE_CONNECTED;
        mqttRetryCount = 0; // Reset retry counter on successful connection
      } else {
        // Start reconnection process
        mqttState = MQTT_STATE_CONNECTING;
        mqttLastAttempt = now;
        sl->Printf("MQTT disconnected. Starting reconnection attempt %d/%d",
                   mqttRetryCount + 1, mqttMaxRetries).Debug();
        sll->Debug("MQTT reconnecting...");
      }
      break;

    case MQTT_STATE_CONNECTING:
      // Only attempt connection once per state entry
      if (now - mqttLastAttempt >= 100) { // Small delay to prevent immediate retry
        // Set server configuration
        IPAddress mqttIP;
        if (mqttIP.fromString(mqttSettings.mqtt_server.get())) {
          client.setServer(mqttIP, static_cast<uint16_t>(mqttSettings.mqtt_port.get()));
        } else {
          client.setServer(mqttSettings.mqtt_server.get().c_str(),
                          static_cast<uint16_t>(mqttSettings.mqtt_port.get()));
        }

        sl->Printf("Attempting MQTT connection to %s:%d (attempt %d/%d)",
                   mqttSettings.mqtt_server.get().c_str(),
                   mqttSettings.mqtt_port.get(),
                   mqttRetryCount + 1, mqttMaxRetries).Debug();

        // Attempt non-blocking connection
        bool connected = client.connect(
          mqttSettings.Publish_Topic.get().c_str(),
          mqttSettings.mqtt_username.get().c_str(),
          mqttSettings.mqtt_password.get().c_str()
        );

        mqttState = MQTT_STATE_WAIT_RESULT;
        mqttLastAttempt = now;
      }
      break;

    case MQTT_STATE_WAIT_RESULT:
      // Check connection result
      if (client.connected()) {
        sl->Debug("MQTT connected successfully!");
        sll->Debug("MQTT connected!");
        mqttState = MQTT_STATE_CONNECTED;
        mqttRetryCount = 0;

        // Subscribe to topics here if needed
        sl->Debug("Ready to subscribe to MQTT topics...");

        //propagate initial boiler Settings to MQTT on startup
        sl->Debug("Propagate initial boiler settings to MQTT...");
        client.publish(mqttSettings.mqtt_Settings_SetState_topic.get().c_str(), String(mqttSettings.mqtt_Settings_SetState.get()).c_str());
        client.publish(mqttSettings.mqtt_Settings_ShowerTime_topic.get().c_str(), String(mqttSettings.mqtt_Settings_ShowerTime.get()).c_str());

      } else if (now - mqttLastAttempt > mqttConnectTimeout) {
        // Connection attempt timed out
        mqttRetryCount++;
        sl->Printf("MQTT connection timeout (rc=%d). Retry %d/%d",
                   client.state(), mqttRetryCount, mqttMaxRetries).Error();

        client.disconnect(); // Clean disconnect

        if (mqttRetryCount >= mqttMaxRetries) {
          mqttState = MQTT_STATE_FAILED;
          sl->Printf("MQTT reconnection failed after %d attempts", mqttMaxRetries).Error();
          sll->Error("MQTT reconnection failed!");
        } else {
          mqttState = MQTT_STATE_IDLE;
          mqttLastAttempt = now; // Reset timer for next attempt
        }
      }
      break;

    case MQTT_STATE_CONNECTED:
      // Continuously check if still connected
      if (!client.connected()) {
        sl->Debug("MQTT connection lost");
        sll->Debug("MQTT connection lost");
        mqttState = MQTT_STATE_IDLE;
      }
      break;

    case MQTT_STATE_FAILED:
      // Wait before trying again (much longer interval after total failure)
      if (now - mqttLastAttempt > 30000) { // Wait 30 seconds after failure
        sl->Debug("Retrying MQTT after failure timeout");
        mqttRetryCount = 0;
        mqttState = MQTT_STATE_IDLE;
      }
      break;
  }
}

void reconnectMQTT()
{
  // Reset state to trigger reconnection
  mqttState = MQTT_STATE_IDLE;
  mqttRetryCount = 0;
}

void publishToMQTT()
{
  if (client.connected())
  {
    sl->Debug("publishToMQTT: Publishing to MQTT...");
    sll->Debug("Publishing to MQTT...");
    client.publish(mqttSettings.mqtt_publish_AktualBoilerTemperature.c_str(), String(temperature).c_str());
    client.publish(mqttSettings.mqtt_publish_AktualTimeRemaining_topic.c_str(), String(boilerTimeRemaining).c_str());
    client.publish(mqttSettings.mqtt_publish_AktualState.c_str(), String(boilerState).c_str());
  }
  else
  {
    sl->Warn("publishToMQTT: MQTT not connected!");
  }
}

void cb_MQTT(char *topic, byte *message, unsigned int length)
{
  String messageTemp((char *)message, length); // Convert byte array to String using constructor
  messageTemp.trim();                          // Remove leading and trailing whitespace

  sl->Printf("<-- MQTT: Topic[%s] <-- [%s]", topic, messageTemp.c_str()).Debug();
  //ToDO: set new Blinker for: helpers.blinkBuidInLED(1, 100); // blink the LED once to indicate that the loop is running
  if (strcmp(topic, mqttSettings.mqtt_Settings_SetState_topic.get().c_str()) == 0)
  {
    // check if it is a number, if not set it to 0
    if (messageTemp.equalsIgnoreCase("null") ||
        messageTemp.equalsIgnoreCase("undefined") ||
        messageTemp.equalsIgnoreCase("NaN") ||
        messageTemp.equalsIgnoreCase("Infinity") ||
        messageTemp.equalsIgnoreCase("-Infinity"))
    {
      sl->Printf("Received invalid value from MQTT: %s", messageTemp.c_str());
      messageTemp = "0";
    }

  }
}

void cb_PublishToMQTT()
{
  publishToMQTT(); // send to Mqtt
}

void cb_MQTTListener()
{
  client.loop(); // process incoming MQTT messages
}

//----------------------------------------
// HELPER FUNCTIONS
//----------------------------------------

void SetupCheckForResetButton()
{
  // check for pressed reset button
  if (digitalRead(buttonSettings.resetDefaultsPin.get()) == LOW)
  {
    sl->Internal("Reset button pressed -> Reset all settings...");
    sll->Internal("Reset button pressed!");
    sll->Internal("Reset all settings!");
    cfg.clearAllFromPrefs(); // Clear all settings from EEPROM
    cfg.saveAll();           // Save the default settings to EEPROM
    
    // Show user feedback that reset is happening
    sll->Internal("Settings reset complete - restarting...");
    
    ESP.restart();           // Restart the ESP32
  }
}

void SetupCheckForAPModeButton()
{
  String APName = "ESP32_Config";
  String pwd = "config1234"; // Default AP password

  // if (wifiSettings.wifiSsid.get().length() == 0 || systemSettings.unconfigured.get())
  if (wifiSettings.wifiSsid.get().length() == 0 )
  {
  sl->Printf("⚠️ SETUP: WiFi SSID is empty [%s] (fresh/unconfigured)", wifiSettings.wifiSsid.get().c_str()).Error();
    cfg.startAccessPoint("192.168.4.1", "255.255.255.0", APName, "");
  }

  // check for pressed AP mode button

  if (digitalRead(buttonSettings.apModePin.get()) == LOW)
  {
  sl->Internal("AP mode button pressed -> starting AP mode...");
  sll->Internal("AP mode button!");
  sll->Internal("-> starting AP mode...");
    cfg.startAccessPoint("192.168.4.1", "255.255.255.0", APName, "");
  }
}

bool SetupStartWebServer()
{
  sl->Printf("⚠️ SETUP: Starting Webserver...!").Debug();
  sll->Printf("Starting Webserver...!").Debug();

  if (wifiSettings.wifiSsid.get().length() == 0)
  {
    sl->Printf("No SSID! --> Start AP!").Debug();
    sll->Printf("No SSID!").Debug();
    sll->Printf("Start AP!").Debug();
    cfg.startAccessPoint();
    // Removed blocking delay(1000);
    return true; // Skip webserver setup if no SSID is set
  }

  if (WiFi.getMode() == WIFI_AP) {
    sl->Printf("🖥️ Run in AP Mode! ");
    sll->Printf("Run in AP Mode! ");
    return false; // Skip webserver setup in AP mode
  }

  if (WiFi.status() != WL_CONNECTED) {
    if (wifiSettings.useDhcp.get())
    {
      sl->Printf("startWebServer: DHCP enabled\n");
      cfg.startWebServer(wifiSettings.wifiSsid.get(), wifiSettings.wifiPassword.get());
    }
    else
    {
      sl->Printf("startWebServer: DHCP disabled\n");
      cfg.startWebServer(wifiSettings.staticIp.get(), wifiSettings.gateway.get(), wifiSettings.subnet.get(),
                  wifiSettings.wifiSsid.get(), wifiSettings.wifiPassword.get());
    }
    // cfg.reconnectWifi();
    WiFi.setSleep(false);
    // Removed blocking delay(1000);
  }
  sl->Printf("\n\nWebserver running at: %s\n", WiFi.localIP().toString().c_str());
  sll->Printf("Web: %s\n\n", WiFi.localIP().toString().c_str());
  sl->Printf("WLAN-Strength: %d dBm\n", WiFi.RSSI());
  sl->Printf("WLAN-Strength is: %s\n\n", WiFi.RSSI() > -70 ? "good" : (WiFi.RSSI() > -80 ? "ok" : "weak"));
  sll->Printf("WLAN: %s\n", WiFi.RSSI() > -70 ? "good" : (WiFi.RSSI() > -80 ? "ok" : "weak"));

  return true; // Webserver setup completed
}

void WriteToDisplay()
{
  // Static variables to track last displayed values
  static float lastTemperature = -999.0;
  static int lastTimeRemaining = -1;
  static bool lastBoilerState = false;
  static bool lastDisplayActive = true;
  
  if (displayActive == false)
  {
    // If display was just turned off, clear it once
    if (lastDisplayActive == true) {
      display.clearDisplay();
      display.display();
      lastDisplayActive = false;
    }
    return; // exit the function if the display is not active
  }
  
  lastDisplayActive = true;
  
  // Only update display if values have changed
  bool needsUpdate = false;
  if (abs(temperature - lastTemperature) > 0.1 || 
      boilerTimeRemaining != lastTimeRemaining || 
      boilerState != lastBoilerState) {
    needsUpdate = true;
    lastTemperature = temperature;
    lastTimeRemaining = boilerTimeRemaining;
    lastBoilerState = boilerState;
  }
  
  if (!needsUpdate) {
    return; // No changes, skip display update
  }

  display.fillRect(0, 0, 128, 24, BLACK); // Clear the previous message area
  display.drawRect(0, 0, 128, 24, WHITE);

  display.setTextSize(1);
  display.setTextColor(WHITE);

  // Show boiler status and temperature
  display.setCursor(3, 3);
  if (temperature > 0) {
    display.printf("Boiler: %s | T:%.1f°C", boilerState ? "ON " : "OFF", temperature);
  } else {
    display.printf("Boiler: %s", boilerState ? "ON " : "OFF");
  }

  // Show remaining time
  display.setCursor(3, 13);
  if (boilerTimeRemaining > 0) {
    display.printf("Time left: %d min", boilerTimeRemaining);
  } else {
    display.printf("Ready");
  }

  display.display();
}

void PinSetup()
{
  analogReadResolution(12);  // Use full 12-bit resolution
  pinMode(buttonSettings.resetDefaultsPin.get(), INPUT_PULLUP);
  pinMode(buttonSettings.apModePin.get(), INPUT_PULLUP);
  Relays::initPins();
  // Force known OFF state
  Relays::setBoiler(false);
}

void CheckButtons()
{
  static bool lastResetButtonState = HIGH;
  static bool lastAPButtonState = HIGH;
  static unsigned long lastButtonCheck = 0;
  
  // Debounce: only check buttons every 50ms
  if (millis() - lastButtonCheck < 50) {
    return;
  }
  lastButtonCheck = millis();
  
  bool currentResetState = digitalRead(buttonSettings.resetDefaultsPin.get());
  bool currentAPState = digitalRead(buttonSettings.apModePin.get());
  
  // Check for button press (transition from HIGH to LOW)
  if (lastResetButtonState == HIGH && currentResetState == LOW)
  {
    sl->Internal("Reset-Button pressed -> Start Display Ticker...");
    ShowDisplay();
  }

  if (lastAPButtonState == HIGH && currentAPState == LOW)
  {
    sl->Internal("AP-Mode-Button pressed -> Start Display Ticker...");
    ShowDisplay();
  }
  
  lastResetButtonState = currentResetState;
  lastAPButtonState = currentAPState;
}

void ShowDisplay()
{
  displayTicker.detach(); // Stop the ticker to prevent multiple calls
  display.ssd1306_command(SSD1306_DISPLAYON); // Turn on the display
  displayTicker.attach(displaySettings.onTimeSec.get(), ShowDisplayOff); // Reattach the ticker to turn off the display after the specified time
  displayActive = true;
}

void ShowDisplayOff()
{
  displayTicker.detach(); // Stop the ticker to prevent multiple calls
  display.ssd1306_command(SSD1306_DISPLAYOFF); // Turn off the display
  // display.fillRect(0, 0, 128, 24, BLACK); // Clear the previous message area

  if (displaySettings.turnDisplayOff.get()){
    displayActive = false;
  }
}

// ------------------------------------------------------------------
// Non-blocking status LED pattern
//  States / patterns:
//   - AP mode: fast blink (100ms on / 100ms off)
//   - Connected STA: slow heartbeat (on 60ms every 2s)
//   - Connecting / disconnected: double blink (2 quick pulses every 1s)
// ------------------------------------------------------------------
void updateStatusLED() {
    static unsigned long lastChange = 0;
    static uint8_t phase = 0;
    unsigned long now = millis();

    bool apMode = WiFi.getMode() == WIFI_AP;
    bool connected = !apMode && WiFi.status() == WL_CONNECTED;

    if (apMode) {
        // simple fast blink 5Hz (100/100)
        if (now - lastChange >= 100) {
            lastChange = now;
            digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
        }
        return;
    }

    if (connected) {
        // heartbeat: brief flash every 2s
        switch (phase) {
            case 0: // LED off idle
                if (now - lastChange >= 2000) { phase = 1; lastChange = now; digitalWrite(LED_BUILTIN, HIGH); }
                break;
            case 1: // LED on briefly
                if (now - lastChange >= 60) { phase = 0; lastChange = now; digitalWrite(LED_BUILTIN, LOW); }
                break;
        }
        return;
    }

    // disconnected / connecting: double blink every ~1s
    switch (phase) {
        case 0: // idle off
            if (now - lastChange >= 1000) { phase = 1; lastChange = now; digitalWrite(LED_BUILTIN, HIGH); }
            break;
        case 1: // first on
            if (now - lastChange >= 80) { phase = 2; lastChange = now; digitalWrite(LED_BUILTIN, LOW); }
            break;
        case 2: // gap
            if (now - lastChange >= 120) { phase = 3; lastChange = now; digitalWrite(LED_BUILTIN, HIGH); }
            break;
        case 3: // second on
            if (now - lastChange >= 80) { phase = 4; lastChange = now; digitalWrite(LED_BUILTIN, LOW); }
            break;
        case 4: // tail gap back to idle
            if (now - lastChange >= 200) { phase = 0; lastChange = now; }
            break;
    }
}

