/*
 * Project: DuinoCoinRig
 * File:    ESP8266_ClientPool
 * Version: 0.1
 * Purpose: Connection and communication with the Duino Coin Pool
 * Author:  Frank Niggemann
 */

#include <ESP8266WiFi.h> // Include WiFi library
#include <ESP8266mDNS.h> // OTA libraries

WiFiClient poolClientInstance[127];
String poolClientBuffer[127];
int poolClientState[127];
unsigned long poolClientTimeRequestStart[127];

#define CLIENT_STATE_UNKNOWN 0
#define CLIENT_STATE_OFFLINE 1
#define CLIENT_STATE_ONLINE 2
#define CLIENT_STATE_JOB_REQUEST_SENT 3
#define CLIENT_STATE_JOB_REQUEST_RESULT 4
#define CLIENT_STATE_JOB_RESULT_SENT 5
#define CLIENT_STATE_JOB_RESULT_RESULT 6
#define CLIENT_STATE_ERROR 7

/**
 * Initializes the pool client part of the software
 */
void clientPoolSetup() {
  for (int id=1 ; id<=127 ; id++) {
    poolClientInstance[id].setTimeout(10000);
    poolClientBuffer[id] = "";
    poolClientState[id] = CLIENT_STATE_UNKNOWN;
    poolClientTimeRequestStart[id] = 0;
  }
}

/**
 * Returns the connection status of the client with the given ID
 * 
 * @param int id The id of the client
 * 
 * @return bool Returns true if client is connected and false if not
 */
bool clientPoolClientIsConnected(int id) {
  if (poolClientInstance[id].connected()) {
    poolClientState[id] = CLIENT_STATE_ONLINE;
    return true;
  }
  poolClientState[id] = CLIENT_STATE_OFFLINE;
  return false;
}

/**
 * Tries to connect the client with the given ID to the pool
 * 
 * @param int id The id of the client
 * 
 * @return bool Returns true if client is connected to the pool and false if not
 */
bool clientPoolConnectClient(int id) {
  if (clientPoolClientIsConnected(id)) {
    return true;
  }
  if (!poolClientInstance[id].connect(serverPoolHost.c_str(), serverPoolPort.toInt())) {
    poolClientState[id] = CLIENT_STATE_OFFLINE;
    return false;
  }
  poolClientState[id] = CLIENT_STATE_ONLINE;
  return true;
}

/**
 * Requests the next job for the client with the given ID
 * 
 * @param int id The id of the client
 * 
 * @return bool Returns true if request is sent and false if not
 */
bool clientPoolRequestNextJobForClient(int id) {
  if (!clientPoolClientIsConnected(id)) {
    return false;
  }
  logMessage("Request next job for client with ID "+String(id));
  poolClientInstance[id].print("JOB," + nameUser + ",AVR");
  poolClientTimeRequestStart[id] = millis();
  poolClientState[id] = CLIENT_STATE_JOB_REQUEST_SENT;
  return true;
}

/**
 * Reads the job result for the client with the given ID
 * 
 * @param int id The id of the client
 * 
 * @return String Returns the result or an empty string
 */
String clientPoolReadJobRequestResultForClient(int id) {
  if (!clientPoolClientIsConnected(id)) {
    return "";
  }
  String result = clientPoolGetContentFromClient(id);
  if (result != "") {
    poolClientState[id] = CLIENT_STATE_JOB_REQUEST_RESULT;
    
  }
  return result;
}

/**
 * Sends the job result for the client with the given ID
 * 
 * @param int id The id of the client
 * 
 * @return bool Returns true if result is sent and false if not
 */
bool clientPoolSendJobResultForClient(int id) {
  if (!clientPoolClientIsConnected(id)) {
    return false;
  }

  return true;
}

/**
 * Reads the result for the job result from the client with the given ID
 * 
 * @param int id The id of the client
 * 
 * @return String Returns the result or an empty string
 */
bool clientPoolReadJobResultResultForClient(int id) {
  if (!clientPoolClientIsConnected(id)) {
    return "";
  }
  String result = clientPoolGetContentFromClient(id);
  if (result != "") {
    poolClientState[id] = CLIENT_STATE_JOB_RESULT_RESULT;
    
  }
  return result;
}

/**
 * Reads and returns the content from the client with the given ID
 * 
 * @param int id The id of the client
 * 
 * @return String The content
 */
String clientPoolGetContentFromClient(int id) {
  String content = "";
  if (poolClientInstance[id].available()) {
    while (poolClientInstance[id].available()) {
      char nextChar = poolClientInstance[id].read();
      if (nextChar != '\n') {
        poolClientBuffer[id] += nextChar;
      } else {
        content = poolClientBuffer[id];
        poolClientBuffer[id] = "";
      }
    }
  }
  return content;
}
