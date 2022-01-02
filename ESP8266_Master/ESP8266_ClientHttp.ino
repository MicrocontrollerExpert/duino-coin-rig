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
  http.setTimeout(30000);
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
