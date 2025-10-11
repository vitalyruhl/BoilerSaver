#pragma once

#include <WiFi.h>
#include <functional>

// WiFi connection states
enum WiFiManagerState {
  WIFI_STATE_DISCONNECTED,
  WIFI_STATE_CONNECTING,
  WIFI_STATE_CONNECTED,
  WIFI_STATE_AP_MODE,
  WIFI_STATE_RECONNECTING
};

// Callback function types
typedef std::function<void()> WiFiConnectedCallback;
typedef std::function<void()> WiFiDisconnectedCallback;
typedef std::function<void()> WiFiAPModeCallback;

class WiFiManager {
private:
  // State management
  WiFiManagerState currentState;
  bool autoRebootEnabled;
  bool initialized;
  
  // Timing variables
  unsigned long lastGoodConnectionMillis;
  unsigned long lastReconnectAttempt;
  unsigned long reconnectInterval;
  unsigned long autoRebootTimeoutMs;
  
  // Callback functions
  WiFiConnectedCallback onConnectedCallback;
  WiFiDisconnectedCallback onDisconnectedCallback;
  WiFiAPModeCallback onAPModeCallback;
  
  // Private methods
  void transitionToState(WiFiManagerState newState);
  void handleReconnection();
  void checkAutoReboot();
  
public:
  // Constructor
  WiFiManager();
  
  // Initialization
  void begin(unsigned long reconnectIntervalMs = 10000, unsigned long autoRebootTimeoutMin = 0);
  void setCallbacks(WiFiConnectedCallback onConnected, WiFiDisconnectedCallback onDisconnected, WiFiAPModeCallback onAPMode = nullptr);
  
  // Main update function (call in loop)
  void update();
  
  // State queries
  WiFiManagerState getState() const;
  bool isConnected() const;
  bool isInAPMode() const;
  unsigned long getLastConnectionTime() const;
  unsigned long getTimeSinceLastConnection() const;
  
  // Control functions
  void enableAutoReboot(bool enable);
  void setAutoRebootTimeout(unsigned long timeoutMinutes);
  void setReconnectInterval(unsigned long intervalMs);
  void forceReconnect();
  void reset();
  
  // Status information
  String getStatusString() const;
  float getConnectionUptime() const; // in seconds
};