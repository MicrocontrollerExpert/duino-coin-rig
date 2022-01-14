/*
 * Project: DuinoCoinRig
 * File:    ESP32_ClientHttps
 * Version: 0.1
 * Purpose: Connection and communication with the Duino Coin Server
 * Author:  Frank Niggemann
 */

#include <HTTPClient.h>
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
  if (serverPoolHost!="" && serverPoolPort!=0) {
    setState(MASTER_STATE_POOL_LOADED);
    logMessage("Updated pool configuration to host " + String(serverPoolHost) + " and port " + String(serverPoolPort));
    serverPoolError = "";
    return;
  }
  String url = "https://server.duinocoin.com/getPool";
  logMessage("Request pool from "+url);
  String content = clientHttpsGetContent(url);
  logMessage("Result: "+content);
  if (content != "") {
    DynamicJsonDocument json(1024);
    logMessage("Result: "+content);
    deserializeJson(json, content);
    const char * jsonIp = json["ip"];
    int jsonPort = json["port"];
    const char * jsonName = json["name"];

    String stringIp((const __FlashStringHelper*) jsonIp);
    String stringName((const __FlashStringHelper*) jsonName);

    serverPoolHost = stringIp;
    serverPoolPort = jsonPort;
    serverPoolName = stringName;

    logMessage("serverPoolHost: "+serverPoolHost);
    logMessage("serverPoolPort: "+String(jsonPort));
    logMessage("serverPoolName: "+serverPoolName);

    setState(MASTER_STATE_POOL_LOADED);
    logMessage("Updated pool configuration to host " + String(serverPoolHost) + " and port " + String(serverPoolPort));
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
    pool = String(serverPoolHost) + ":" + String(serverPoolPort);
  }
  return pool;
}
