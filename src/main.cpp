#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <ArduinoOTA.h>
#include <WebSocketsServer.h>
#include "main.h"
#include <time.h>

void setup()
{
  pinMode(motionSensePin, INPUT);
  pinMode(motionSensePin_2, INPUT);

  Serial.begin(115200); // Start the Serial communication to send messages to the computer
  delay(10);
  Serial.println("\r\n");

  startWiFi(); // Start a Wi-Fi access point, and try to connect to some given access points. Then wait for either an AP or STA connection

  startOTA(); // Start the OTA service

  startWebSocket(); // Start a WebSocket server
}

void loop()
{
  webSocket.loop();    // constantly check for websocket events
  ArduinoOTA.handle(); // listen for OTA events
  handleMotionStatus();
}

void startWiFi()
{
  WiFi.begin("Orange_Swiatlowod_Gora", "mlekogrzybowe"); // add Wi-Fi networks you want to connect to

  Serial.println("Connecting");

  unsigned long startTime = millis();
  bool connectionFailed = false;

  while (WiFi.status() != WL_CONNECTED && !connectionFailed)
  { // Wait for the Wi-Fi to connect
    delay(250);
    Serial.print('.');
    connectionFailed = (millis() - startTime < delayBeforeAP) ? false : true;
  }

  if (connectionFailed)
  {
    WiFi.softAP(ssid, password); // Start the access point
    Serial.print("Access Point \"");
    Serial.print(ssid);
    Serial.println("\" started\r\n");
  }
  else
  {
    Serial.println("\r\n");

    Serial.println("\r\n");

    WiFi.setAutoReconnect(true);
    WiFi.persistent(true);
  }
}

void startOTA()
{
  // Start the OTA service
  ArduinoOTA.setHostname(OTAName);
  ArduinoOTA.setPassword(OTAPassword);

  ArduinoOTA.onStart([]()
                     { Serial.println("Start"); });
  ArduinoOTA.onEnd([]()
                   { Serial.println("\r\nEnd"); });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total)
                        { Serial.printf("Progress: %u%%\r", (progress / (total / 100))); });
  ArduinoOTA.onError([](ota_error_t error)
                     {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
    else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
    else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
    else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
    else if (error == OTA_END_ERROR) Serial.println("End Failed"); });
  ArduinoOTA.begin();
  Serial.println("OTA ready\r\n");
}

void startWebSocket()
{                                    // Start a WebSocket server
  webSocket.begin();                 // start the websocket server
  webSocket.onEvent(webSocketEvent); // if there's an incomming websocket message, go to function 'webSocketEvent'
  Serial.println("WebSocket server started.");
}

void webSocketEvent(uint8_t num, WStype_t type, uint8_t *payload, size_t lenght)
{
  // When a WebSocket message is received
  // check motion sensor status and provide it
  String motionSenseStatus = String((digitalRead(motionSensePin)));

  switch (type)
  {
  case WStype_DISCONNECTED:
  {

    Serial.printf("[%u] Disconnected!\n", num);
    break;
  }
  case WStype_CONNECTED:
  {
    IPAddress ip = webSocket.remoteIP(num);
    Serial.printf("[%u] Connected from %d.%d.%d.%d url: %s\n", num, ip[0], ip[1], ip[2], ip[3], payload);
    Serial.printf("broadcastTXT status=%s\n", motionSenseStatus);
    webSocket.broadcastTXT(motionSenseStatus);
  }
  break;
  case WStype_TEXT:
  {
    Serial.printf("[%u] got WS text: %s\n", num, payload);
    Serial.printf("broadcastTXT status=%s\n", motionSenseStatus);
    webSocket.broadcastTXT(motionSenseStatus);
  }
  }
}

// Function to set the pin state and hold it for a specified duration in milliseconds
void holdPinState(int pinState, int duration)
{
  // Set the pin state here
  // For example: digitalWrite(pin, pinState);
  static uint64_t holdStartTime;
  // Hold the state for the specified duration
  if ((millis() - holdStartTime) < duration)
  {
    uint64_t holdStartTime = millis();
    Serial.printf("Hold pin state for %i ms \r\n", (millis() - holdStartTime));
    digitalWrite(motionSensePin, 0x1); // Set the pin state here
  }
  else
  {
    // duration passed
    Serial.println("Hold time passed");
    digitalWrite(motionSensePin, 0x0); // Set the pin state here
  }
}


void handleMotionStatus()
{
  uint64_t currentMillis = millis();
  if (currentMillis - lastCheckTime >= interval)
  {
    // Save the last time the sensor was checked
    lastCheckTime = currentMillis;

    // hold status for some time
    uint8_t pinState = digitalRead(motionSensePin);
    if (pinState)
    {
      // Hold the pin state for 1000ms
      holdPinState(pinState, 10000);
    }
#if 0
    Serial.printf("currentMillis=%i \t D0 status= %i \t D1 status= %i \r\n", 
    (uint32_t)currentMillis, digitalRead(motionSensePin), digitalRead(motionSensePin_2));
#endif

    // Only if state changed
    if (digitalRead(motionSensePin) != lastCheckStatus)
    {
      lastCheckStatus = digitalRead(motionSensePin);

      // set led
      digitalWrite(D0, lastCheckStatus);

      // send message to websocket
      String motionSenseStatusString = String(lastCheckStatus);

      Serial.printf("motionSenseStatusString=%s \r\n", motionSenseStatusString);

      webSocket.broadcastTXT(motionSenseStatusString);
    }
  }
}