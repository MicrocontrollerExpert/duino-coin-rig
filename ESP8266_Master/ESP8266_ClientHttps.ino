/*
 * Project: DuinoCoinRig
 * File:    ESP8266_ClientHttps
 * Version: 0.1
 * Purpose: Connection and communication with the Duino Coin Server
 * Author:  Frank Niggemann
 */

#include <ESP8266HTTPClient.h>
#include <WiFiClientSecure.h>
#include <ArduinoJson.h>

/**
 * Initializes the HTTPS client part of the software
 */
void clientHttpsSetup() {
 
}

/**
 * Reads and returns the content from the given URL
 * 
 * @param String url The URL whose content is to be read and returned
 * 
 * @return String The content
 */
String clientHttpsGetContent(String url) {
  String content = "";
  WiFiClientSecure client;
  client.setInsecure();
  
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

/**
 * Requests a pool configuration from the pool server
 */ 
void clientHttpsRequestPoolConfiguration() {
  setState(MASTER_STATE_LOADING_POOL);
  String url = "https://server.duinocoin.com/getPool";
  logMessage("Request pool from "+url);
  String content = clientHttpsGetContent(url);
  if (content != "") {
    DynamicJsonDocument json(1024);
    deserializeJson(json, content);
    serverPoolHost = String(json["ip"]);
    serverPoolPort = String(json["port"]);
    serverPoolName = String(json["name"]);
    logMessage("Updated pool configuration to host " + serverPoolHost + " and port " + serverPoolPort);
    serverPoolError = "";
  } else {
    serverPoolError = "Connection failed!";
  }
}

/**
 * Returns the pool configuration or 'unknown' as result
 * 
 * @return String The pool configuration or 'unknown'
 */
String clientHttpsGetPoolString() {
  String pool = "Unknown";
  if (serverPoolError != "") {
    return serverPoolError;
  }
  if (serverPoolHost != "") {
    pool = serverPoolHost + ":" + serverPoolPort;
  }
  return pool;
}
