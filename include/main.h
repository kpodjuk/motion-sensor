#include <string.h>

// settings
const int motionSensePin = D1;   // gpio 0, d0
const int motionSensePin_2 = D2; // gpio 1, d1
const unsigned long interval = 50;
const uint64_t delayBeforeAP = 5 * 60 * 1000;
const char *ssid = "Motion sensor"; // The name of the Wi-Fi network that will be created
const char *password = "";          // The password required to connect to it, leave blank for an open network
const char *OTAName = "ESP8266";    // A name and a password for the OTA service
const char *OTAPassword = "esp8266";

ESP8266WiFiMulti wifiMulti;     // Create an instance of the ESP8266WiFiMulti class, called 'wifiMulti'
WebSocketsServer webSocket(81); // create a websocket server on port 81
uint64_t lastCheckTime;
bool lastCheckStatus;

// ************ Function definitions ************
void startWiFi();      // Start a Wi-Fi access point, and try to connect to some given access points. Then wait for either an AP or STA connection
void startOTA();       // Start the OTA service
void startWebSocket(); // Start a WebSocket server
void webSocketEvent(uint8_t num, WStype_t type, uint8_t *payload, size_t lenght);
void handleMotionStatus();