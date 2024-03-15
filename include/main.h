#include <string.h>

// settings
const int motionSensePin = 0; // gpio 0

// Defines that help with colorizing serial output ###
#define Black "\e[0;30m"
#define Red "\e[0;31m"
#define Green "\e[0;32m"
#define Yellow "\e[0;33m"
#define Blue "\e[0;34m"
#define Purple "\e[0;35m"
#define Cyan "\e[0;36m"
#define White "\e[0;37m"
#define EndColor "\e[0"

ESP8266WiFiMulti wifiMulti;         // Create an instance of the ESP8266WiFiMulti class, called 'wifiMulti'
WebSocketsServer webSocket(81);     // create a websocket server on port 81
const char *ssid = "Motion sensor"; // The name of the Wi-Fi network that will be created
const char *password = "";          // The password required to connect to it, leave blank for an open network
const char *OTAName = "ESP8266";    // A name and a password for the OTA service
const char *OTAPassword = "esp8266";

// ************ Function definitions ************
void startWiFi();      // Start a Wi-Fi access point, and try to connect to some given access points. Then wait for either an AP or STA connection
void startOTA();       // Start the OTA service
void startWebSocket(); // Start a WebSocket server
void webSocketEvent(uint8_t num, WStype_t type, uint8_t *payload, size_t lenght);
void sendMotionSenseStatus();