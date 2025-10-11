#include "wifi_manager.h"
#include <ESP.h>

WiFiManager::WiFiManager() 
  : currentState(WIFI_STATE_DISCONNECTED)
  , autoRebootEnabled(false)
  , initialized(false)
  , lastGoodConnectionMillis(0)
  , lastReconnectAttempt(0)
  , reconnectInterval(10000)
  , autoRebootTimeoutMs(0)
  , onConnectedCallback(nullptr)
  , onDisconnectedCallback(nullptr)
  , onAPModeCallback(nullptr)
{
}

void WiFiManager::begin(unsigned long reconnectIntervalMs, unsigned long autoRebootTimeoutMin) {
  reconnectInterval = reconnectIntervalMs;
  autoRebootTimeoutMs = autoRebootTimeoutMin * 60000UL; // Convert minutes to milliseconds
  autoRebootEnabled = (autoRebootTimeoutMin > 0);
  
  // Initialize timing
  lastGoodConnectionMillis = millis();
  lastReconnectAttempt = 0;
  
  // Determine initial state
  if (WiFi.getMode() == WIFI_AP) {
    currentState = WIFI_STATE_AP_MODE;
  } else if (WiFi.status() == WL_CONNECTED) {
    currentState = WIFI_STATE_CONNECTED;
    lastGoodConnectionMillis = millis();
  } else {
    currentState = WIFI_STATE_DISCONNECTED;
  }
  
  initialized = true;
}

void WiFiManager::setCallbacks(WiFiConnectedCallback onConnected, WiFiDisconnectedCallback onDisconnected, WiFiAPModeCallback onAPMode) {
  onConnectedCallback = onConnected;
  onDisconnectedCallback = onDisconnected;
  onAPModeCallback = onAPMode;
}

void WiFiManager::update() {
  if (!initialized) return;
  
  WiFiManagerState previousState = currentState;
  
  // Determine current WiFi state
  if (WiFi.getMode() == WIFI_AP) {
    if (currentState != WIFI_STATE_AP_MODE) {
      transitionToState(WIFI_STATE_AP_MODE);
    }
  } else if (WiFi.status() == WL_CONNECTED) {
    if (currentState != WIFI_STATE_CONNECTED) {
      transitionToState(WIFI_STATE_CONNECTED);
    }
    // Update last good connection time
    lastGoodConnectionMillis = millis();
  } else {
    // WiFi is disconnected
    if (currentState == WIFI_STATE_CONNECTED) {
      transitionToState(WIFI_STATE_DISCONNECTED);
    }
    
    // Handle reconnection attempts
    handleReconnection();
  }
  
  // Check auto-reboot condition
  if (autoRebootEnabled && currentState != WIFI_STATE_AP_MODE) {
    checkAutoReboot();
  }
}

void WiFiManager::transitionToState(WiFiManagerState newState) {
  WiFiManagerState oldState = currentState;
  currentState = newState;
  
  // Execute callbacks based on state transitions
  switch (newState) {
    case WIFI_STATE_CONNECTED:
      if (oldState != WIFI_STATE_CONNECTED && onConnectedCallback) {
        onConnectedCallback();
      }
      break;
      
    case WIFI_STATE_DISCONNECTED:
    case WIFI_STATE_RECONNECTING:
      if (oldState == WIFI_STATE_CONNECTED && onDisconnectedCallback) {
        onDisconnectedCallback();
      }
      break;
      
    case WIFI_STATE_AP_MODE:
      if (oldState != WIFI_STATE_AP_MODE && onAPModeCallback) {
        onAPModeCallback();
      }
      break;
      
    default:
      break;
  }
}

void WiFiManager::handleReconnection() {
  if (WiFi.getMode() == WIFI_AP) return; // Don't reconnect in AP mode
  
  unsigned long now = millis();
  
  if (now - lastReconnectAttempt >= reconnectInterval) {
    lastReconnectAttempt = now;
    
    if (currentState != WIFI_STATE_RECONNECTING) {
      transitionToState(WIFI_STATE_RECONNECTING);
    }
    
    // Attempt non-blocking reconnection
    WiFi.reconnect();
  }
}

void WiFiManager::checkAutoReboot() {
  if (!autoRebootEnabled || autoRebootTimeoutMs == 0) return;
  
  unsigned long now = millis();
  unsigned long timeSinceLastConnection = now - lastGoodConnectionMillis;
  
  if (timeSinceLastConnection >= autoRebootTimeoutMs) {
    // Time for auto-reboot
    ESP.restart();
  }
}

WiFiManagerState WiFiManager::getState() const {
  return currentState;
}

bool WiFiManager::isConnected() const {
  return currentState == WIFI_STATE_CONNECTED;
}

bool WiFiManager::isInAPMode() const {
  return currentState == WIFI_STATE_AP_MODE;
}

unsigned long WiFiManager::getLastConnectionTime() const {
  return lastGoodConnectionMillis;
}

unsigned long WiFiManager::getTimeSinceLastConnection() const {
  return millis() - lastGoodConnectionMillis;
}

void WiFiManager::enableAutoReboot(bool enable) {
  autoRebootEnabled = enable;
}

void WiFiManager::setAutoRebootTimeout(unsigned long timeoutMinutes) {
  autoRebootTimeoutMs = timeoutMinutes * 60000UL;
  autoRebootEnabled = (timeoutMinutes > 0);
}

void WiFiManager::setReconnectInterval(unsigned long intervalMs) {
  reconnectInterval = intervalMs;
}

void WiFiManager::forceReconnect() {
  lastReconnectAttempt = 0; // Reset timer to trigger immediate reconnect
}

void WiFiManager::reset() {
  currentState = WIFI_STATE_DISCONNECTED;
  lastGoodConnectionMillis = millis();
  lastReconnectAttempt = 0;
}

String WiFiManager::getStatusString() const {
  switch (currentState) {
    case WIFI_STATE_CONNECTED:
      return "Connected";
    case WIFI_STATE_CONNECTING:
      return "Connecting";
    case WIFI_STATE_DISCONNECTED:
      return "Disconnected";
    case WIFI_STATE_AP_MODE:
      return "AP Mode";
    case WIFI_STATE_RECONNECTING:
      return "Reconnecting";
    default:
      return "Unknown";
  }
}

float WiFiManager::getConnectionUptime() const {
  if (currentState == WIFI_STATE_CONNECTED) {
    return (millis() - lastGoodConnectionMillis) / 1000.0f;
  }
  return 0.0f;
}