/*
 * Project: DuinoCoinRig
 * File:    ESP8266_ClientHttp
 * Version: 0.1
 * Purpose: Connection and communication with the Duino Coin Server
 * Author:  Frank Niggemann
 */

#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>
#include <ArduinoJson.h>

/**
 * Initializes the HTTP client part of the software
 */
void clientHttpSetup() {
  clientHttpRequestPoolConfiguration();
}

/**
 * Reads and returns the content from the given URL
 * 
 * @param String url The URL whose content is to be read and returned
 * 
 * @return String The content
 */
String clientHttpGetContent(String url) {
  String content = "";
  WiFiClient client;
  HTTPClient http;
  if (http.begin(client, url))
  {
    int httpCode = http.GET();
    if (httpCode == HTTP_CODE_OK)
    {
      content = http.getString();
    }
    else
    {
      logMessage("HTTP get content failed with error: " + http.errorToString(httpCode));
    }
    http.end();
  }
  return content;
}

/**
 * Requests a pool configuration from the pool server
 */ 
void clientHttpRequestPoolConfiguration() {
  String url = "http://" + serverPoolRequestHost + ":" + serverPoolRequestPort + "/getPool";
  String content = clientHttpGetContent(url);
  if (content != "") {
    DynamicJsonDocument json(1024);
    deserializeJson(json, content);
    serverPoolHost = String(json["ip"]);
    serverPoolPort = String(json["port"]);
    serverPoolName = String(json["name"]);
    logMessage("Updated pool configuration to host " + serverPoolHost + " and port " + serverPoolPort);
  }
}

/**
 * Returns the pool configuration or 'unknown' as result
 * 
 * @return String The pool configuration or 'unknown'
 */
String clientHttpGetPoolString() {
  String pool = "Unknown";
  if (serverPoolHost != "") {
    pool = serverPoolHost + ":" + serverPoolPort;
  }
  return pool;
}
