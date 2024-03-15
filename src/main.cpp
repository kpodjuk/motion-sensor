#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <ArduinoOTA.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <FS.h>
#include <WebSocketsServer.h>
#include "ArduinoJson.h"
#include "main.h"
#include <arduino-timer.h>
// WebSocketsServer webSocket = WebSocketsServer(81);

void setup()
{
  pinMode(0, INPUT);

  Serial.begin(115200); // Start the Serial communication to send messages to the computer
  delay(10);
  Serial.println("\r\n");

  startWiFi(); // Start a Wi-Fi access point, and try to connect to some given access points. Then wait for either an AP or STA connection

  startOTA(); // Start the OTA service
  // startSPIFFS(); // Start the SPIFFS and list all contents

  // startWebSocket(); // Start a WebSocket server
  // startServer();    // Start a HTTP server with a file read handler and an upload handler
  webSocket.begin();
  webSocket.onEvent(webSocketEvent);
}
#define RAPORT_WIFI_STATUS

// void hexdump(const void *mem, uint32_t len, uint8_t cols = 16)
// {
//   const uint8_t *src = (const uint8_t *)mem;
//   Serial.printf("\n[HEXDUMP] Address: 0x%08X len: 0x%X (%d)", (ptrdiff_t)src, len, len);
//   for (uint32_t i = 0; i < len; i++)
//   {
//     if (i % cols == 0)
//     {
//       Serial.printf("\n[0x%08X] 0x%08X: ", (ptrdiff_t)src, i);
//     }
//     Serial.printf("%02X ", *src);
//     src++;
//   }
//   Serial.printf("\n");
// }

void loop()
{

#ifdef RAPORT_WIFI_STATUS
  raportStatusOnSerial();
#endif
  // server.handleClient(); // run the server
  ArduinoOTA.handle(); // listen for OTA events

  webSocket.loop(); // constantly check for websocket events
}

void startWiFi()
{ // Start a Wi-Fi access point, and try to connect to some given access points. Then wait for either an AP or STA connection
  // WiFi.softAP(ssid, password); // Start the access point
  // Serial.print("Access Point \"");
  // Serial.print(ssid);
  // Serial.println("\" started\r\n");

  // will it connect to multiple networks or only one?
  WiFi.begin("Orange_Swiatlowod_Gora", "mlekogrzybowe"); // add Wi-Fi networks you want to connect to
  // wifiMulti.addAP("Orange_Swiatlowod_E8A0", "mlekogrzybowe);

  Serial.println("Connecting");
  while (WiFi.status() != WL_CONNECTED)
  { // Wait for the Wi-Fi to connect
    delay(250);
    Serial.print('.');
  }
  Serial.println("\r\n");

  Serial.println("\r\n");

  WiFi.setAutoReconnect(true);
  WiFi.persistent(true);
}

void startOTA()
{ // Start the OTA service
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

void startSPIFFS()
{                 // Start the SPIFFS and list all contents
  SPIFFS.begin(); // Start the SPI Flash File System (SPIFFS)
  Serial.println("SPIFFS started. Contents:");
  {
    Dir dir = SPIFFS.openDir("/");
    while (dir.next())
    { // List the file system contents
      String fileName = dir.fileName();
      size_t fileSize = dir.fileSize();
      Serial.printf("\tFS File: %s, size: %s\r\n", fileName.c_str(), formatBytes(fileSize).c_str());
    }
    Serial.printf("\n");
  }
}

void startWebSocket()
{                                    // Start a WebSocket server
  webSocket.begin();                 // start the websocket server
  webSocket.onEvent(webSocketEvent); // if there's an incomming websocket message, go to function 'webSocketEvent'
  Serial.println("WebSocket server started.");
}

void startServer()
{                                           // Start a HTTP server with a file read handler and an upload handler
  server.on("/edit.html", HTTP_POST, []() { // If a POST request is sent to the /edit.html address,
    server.send(200, "text/plain", "");
  });

  server.on("/api", HTTP_GET, []()
            {
    if (server.argName(0) == "p")
    {
      String desiredAction = server.arg(0);
      String answer;
      if (desiredAction == "on")
      {
        Serial.println("\e[0;35m/api: Turning on!\e[0m");
        answer = "/api: Turning on if required!";
        server.send(200, "text/plain", answer);
      }
      else if (desiredAction == "off")
      {
        Serial.println("\e[0;35m/api: Turning off!\e[0m");
        server.send(200, "text/plain", "/api: Turning off!");
      }
      else if (desiredAction == "toggle")
      {
        Serial.println("\e[0;35m/api: Toggling!\e[0m");
        server.send(200, "text/plain", "/api: Toggling!");
      }
      else
      {
        Serial.println("\e[0;31m/api: Wrong param value!\e[0m");
        server.send(200, "text/plain", "/api: Wrong param value!");
      }
    } 
    else
    {
      // if no param = send current PC status
      Serial.println("\e[0;35m/api: Sending PC status!\e[0m");
      char answerCharArr[25];
      sprintf(answerCharArr, "/api: PC status: %s", 
      digitalRead(powerLightPin) == 0 ? "Online" : "Offline");
      server.send(200, "text/plain", answerCharArr);
    } });

  server.onNotFound(handleNotFound); // if someone requests any other file or page, go to function 'handleNotFound'
                                     // and check if the file exists

  server.begin(); // start the HTTP server
  Serial.println("HTTP server started.");
}

void handleNotFound()
{
// if the requested file or page doesn't exist, return a 404 not found error
#ifdef NO_WEBCLIENT
  server.send(404, "text/plain", "404: WebClient not available in this build, use /api?p=");
#else
  if (!handleFileRead(server.uri()))
  { // check if the file exists in the flash memory (SPIFFS), if so, send it
    server.send(404, "text/plain", "404: File Not Found");
  }
#endif
}

bool handleFileRead(String path)
{ // send the right file to the client (if it exists)
  Serial.println("handleFileRead: " + path);
  if (path.endsWith("/"))
    path += "index.html";                    // If a folder is requested, send the index file
  String contentType = getContentType(path); // Get the MIME type
  String pathWithGz = path + ".gz";
  if (SPIFFS.exists(pathWithGz) || SPIFFS.exists(path))
  {                                                     // If the file exists, either as a compressed archive, or normal
    if (SPIFFS.exists(pathWithGz))                      // If there's a compressed version available
      path += ".gz";                                    // Use the compressed verion
    File file = SPIFFS.open(path, "r");                 // Open the file
    size_t sent = server.streamFile(file, contentType); // Send it to the client
    file.close();                                       // Close the file again
    Serial.println(String("\tSent file-> ") + path);
    return true;
  }
  Serial.println(String("\tFile Not Found: ") + path); // If the file doesn't exist, return false
  return false;
}
void webSocketEvent(uint8_t num, WStype_t type, uint8_t *payload, size_t length)
{

  switch (type)
  {
  case WStype_DISCONNECTED:
    Serial.printf("[%u] Disconnected!\n", num);
    break;
  case WStype_CONNECTED:
  {
    IPAddress ip = webSocket.remoteIP(num);
    Serial.printf("[%u] Connected from %d.%d.%d.%d url: %s\n", num, ip[0], ip[1], ip[2], ip[3], payload);

    // send message to client
    webSocket.sendTXT(num, "Connected");
  }
  break;
  case WStype_TEXT:
    Serial.printf("[%u] get Text: %s\n", num, payload);

    // send message to client
    // webSocket.sendTXT(num, "message here");

    // send data to all connected clients
    // webSocket.broadcastTXT("message here");
    break;
  case WStype_BIN:
    Serial.printf("[%u] get binary length: %u\n", num, length);
    // hexdump(payload, length);

    // send message to client
    // webSocket.sendBIN(num, payload, length);
    break;
  case WStype_ERROR:
  case WStype_FRAGMENT_TEXT_START:
  case WStype_FRAGMENT_BIN_START:
  case WStype_FRAGMENT:
  case WStype_FRAGMENT_FIN:
    break;
  }
}

String formatBytes(size_t bytes)
{ // convert sizes in bytes to KB and MB
  if (bytes < 1024)
  {
    return String(bytes) + "B";
  }
  else if (bytes < (1024 * 1024))
  {
    return String(bytes / 1024.0) + "KB";
  }
  else if (bytes < (1024 * 1024 * 1024))
  {
    return String(bytes / 1024.0 / 1024.0) + "MB";
  }
  return String(0);
}

String getContentType(String filename)
{ // determine the filetype of a given filename, based on the extension
  if (filename.endsWith(".html"))
    return "text/html";
  else if (filename.endsWith(".css"))
    return "text/css";
  else if (filename.endsWith(".js"))
    return "application/javascript";
  else if (filename.endsWith(".ico"))
    return "image/x-icon";
  else if (filename.endsWith(".gz"))
    return "application/x-gzip";
  return "text/plain";
}

void sendStatus()
{
  // send current status to websocket client here (mode, settings for that mode)

  jsonDoc["turnedOn"] = !digitalRead(powerLightPin);

  String statusString;
  serializeJson(jsonDoc, statusString);

  Serial.print("Sending status:");

  Serial.print(statusString);

  // It's sent to every client connected, not the one who requested it
  // no harm in that tho
  webSocket.broadcastTXT(statusString);
}

void pressPowerButton()
{
  const uint16_t delayBetweenPresses = 200;

  // Pin LOW == you pressed power button
  digitalWrite(powerButtonPin, LOW);
  Serial.printf("\e[0;33mpowerButtonPin=%i\e[0m\n", LOW);
  delay(delayBetweenPresses); // too small of a delay and it might not work
  digitalWrite(powerButtonPin, HIGH);
  Serial.printf("\e[0;33mpowerButtonPin=%i\e[0m\n", HIGH);

#ifndef NO_WEBCLIENT
  sendStatus();
#endif
}

void raportStatusOnSerial()
{

  unsigned long currentMillis = millis();

  if (currentMillis - previousMillis >= interval)
  {
    previousMillis = currentMillis;

    Serial.printf("\e[0;34m ############### Info ############### \e[0m \n");

    Serial.printf("Signal strength: %i dBm\n", WiFi.RSSI());

    Serial.print("SSID: ");
    Serial.println(WiFi.SSID());

    Serial.print("localIP: ");
    Serial.println(WiFi.localIP());

    Serial.print("Status: ");

    char color[20] = "\e[0;31m"; // red by default
    switch (WiFi.status())
    {
    case WL_IDLE_STATUS:
    {
      Serial.printf("%sWL_IDLE_STATUS \e[0m\n", color);
      break;
    }
    case WL_NO_SSID_AVAIL:
    {
      Serial.printf("%sWL_NO_SSID_AVAIL \e[0m\n", color);
      break;
    }
    case WL_SCAN_COMPLETED:
    {
      Serial.printf("%sWL_SCAN_COMPLETED \e[0m\n", color);
      break;
    }
    case WL_CONNECTED:
    {
      strcpy(color, "\e[0;32m"); // change to green when connected
      Serial.printf("%sWL_CONNECTED \e[0m\n", color);
      break;
    }
    case WL_CONNECT_FAILED:
    {
      Serial.printf("%sWL_CONNECT_FAILED \e[0m\n", color);
      break;
    }
    case WL_CONNECTION_LOST:
    {
      Serial.printf("%sWL_CONNECTION_LOST \e[0m\n", color);
      break;
    }
    case WL_DISCONNECTED:
    {
      Serial.printf("%sWL_DISCONNECTED \e[0m\n", color);
      break;
    }
    }
  }
}