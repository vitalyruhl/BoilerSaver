
#include "settings.h"

ConfigManagerClass cfg;
WiFi_Settings wifiSettings;
MQTT_Settings mqttSettings;

I2CSettings i2cSettings;

DisplaySettings displaySettings;
SystemSettings systemSettings;
ButtonSettings buttonSettings;

BoilerSettings boilerSettings;

SigmaLogLevel logLevel = SIGMALOG_WARN; // SIGMALOG_OFF = 0, SIGMALOG_INTERNAL, SIGMALOG_FATAL, SIGMALOG_ERROR, SIGMALOG_WARN, SIGMALOG_INFO, SIGMALOG_DEBUG, SIGMALOG_ALL
ConfigManagerClass::LogCallback ConfigManagerClass::logger = nullptr;
