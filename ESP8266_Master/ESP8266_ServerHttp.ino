/*
 * Project: DuinoCoinRig
 * File:    ESP8266_ServerHttp
 * Version: 0.1
 * Purpose: The local HTTP server for the user front end
 * Author:  Frank Niggemann
 */

#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <ESPAsyncTCP.h>
#include <SPIFFSEditor.h>
#include <ESPAsyncWebServer.h>

AsyncWebServer server(80);
AsyncWebSocket ws("/ws");

const char* http_username = "admin";
const char* http_password = "admin";

void onWsEvent(AsyncWebSocket * server, AsyncWebSocketClient * client, AwsEventType type, void * arg, uint8_t *data, size_t len);

/**
 * Initializes the HTTP server part of the software
 */
void serverHttpSetup() {
  ws.onEvent(onWsEvent);
  server.addHandler(&ws);
  SPIFFS.begin();
  server.addHandler(new SPIFFSEditor(http_username, http_password));

  server.on("/api/status", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send(200, "text/plain", serverHttpApiStatus());
  });

  server.on("/api/log", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send(200, "text/plain", serverHttpApiLog());
  });
  
  server.serveStatic("/", SPIFFS, "/").setDefaultFile("index.html");

  server.begin();
}

/**
 * Returns the current rig status
 */
String serverHttpApiStatus() {
  String content = "Status";

  return content;
}

/**
 * Returns the log files
 */
String serverHttpApiLog() {
  String content = "Log";

  return content;
}
