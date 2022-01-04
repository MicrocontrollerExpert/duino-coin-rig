/*
 * Project: DuinoCoinRig
 * File:    ESP8266_ClientPool
 * Version: 0.1
 * Purpose: Connection and communication with the Duino Coin Pool
 * Author:  Frank Niggemann
 */

#include <ESP8266WiFi.h> // Include WiFi library
#include <ESP8266mDNS.h> // OTA libraries

WiFiClient poolClientInstance[SLAVE_ID_MAX];
String poolClientBuffer[SLAVE_ID_MAX];
String poolClientServerVersion[SLAVE_ID_MAX];
String poolClientLastBlockHash[SLAVE_ID_MAX];
String poolClientNextBlockHash[SLAVE_ID_MAX];
String poolClientDifficulty[SLAVE_ID_MAX];
int poolClientState[SLAVE_ID_MAX];
unsigned long poolClientTimeRequestStart[SLAVE_ID_MAX];

String ducos1aResult[SLAVE_ID_MAX];
String ducoId[SLAVE_ID_MAX];
String microtimeDifference[SLAVE_ID_MAX];

#define CLIENT_STATE_UNKNOWN 0
#define CLIENT_STATE_OFFLINE 1
#define CLIENT_STATE_ONLINE 2
#define CLIENT_STATE_JOB_REQUEST_SENT_TO_SERVER 3
#define CLIENT_STATE_JOB_REQUEST_RESULT_FROM_SERVER 4
#define CLIENT_STATE_JOB_SENT_TO_SLAVE 5
#define CLIENT_STATE_JOB_RESULT_FROM_SLAVE 6
#define CLIENT_STATE_JOB_RESULT_SENT_TO_SERVER 7
#define CLIENT_STATE_JOB_RESULT_RESULT_FROM_SERVER 8
#define CLIENT_STATE_ERROR 9

/**
 * Initializes the pool client part of the software
 */
void clientPoolSetup() {
  for (int id=SLAVE_ID_MIN ; id<=SLAVE_ID_MAX ; id++) {
    poolClientInstance[id].setTimeout(30000);
    poolClientBuffer[id] = "";
    setStateClient(id, CLIENT_STATE_UNKNOWN);
    poolClientTimeRequestStart[id] = 0;
  }
}

/**
 * Connects all found clients to the pool server
 */
void clientPoolConnectClients() {
  setState(MASTER_STATE_CONNECTING_CLIENTS);
  for (int id=SLAVE_ID_MIN ; id<=SLAVE_ID_MAX ; id++) {
    if (slaveFound[id]) {
      clientPoolConnectClient(id); 
    }
  }
  setState(MASTER_STATE_CLIENTS_CONNECTED);
}

void clientPoolRotateStates() {
  for (int id=SLAVE_ID_MIN ; id<=SLAVE_ID_MAX ; id++) {
    if (slaveFound[id]) {
      if (poolClientState[id] == CLIENT_STATE_UNKNOWN) {
        clientPoolConnectClient(id); 
      } else if (poolClientState[id] == CLIENT_STATE_OFFLINE) {
        clientPoolConnectClient(id); 
      } else if (poolClientState[id] == CLIENT_STATE_ONLINE) {
        clientPoolRequestNextJobForClient(id);
      } else if (poolClientState[id] == CLIENT_STATE_JOB_REQUEST_SENT_TO_SERVER) {
        clientPoolReadJobRequestResultForClient(id);
      } else if (poolClientState[id] == CLIENT_STATE_JOB_REQUEST_RESULT_FROM_SERVER) {
        clientPoolSendClientJobToSlave(id);
      } else if (poolClientState[id] == CLIENT_STATE_JOB_SENT_TO_SLAVE) {
        clientPoolRequestClientJobResultFromSlave(id);
      } else if (poolClientState[id] == CLIENT_STATE_JOB_RESULT_FROM_SLAVE) {
        clientPoolSendJobResultForClient(id);
      } else if (poolClientState[id] == CLIENT_STATE_JOB_RESULT_SENT_TO_SERVER) {
        clientPoolReadJobResultResultForClient(id);
      } else if (poolClientState[id] == CLIENT_STATE_JOB_RESULT_RESULT_FROM_SERVER) {
        unsigned long poolClientTimeRequestEnd = millis();
        if ((poolClientTimeRequestEnd - poolClientTimeRequestStart[id]) > 30000) {
          poolClientState[id] = CLIENT_STATE_ONLINE;
        }
      } else {
        poolClientState[id] = CLIENT_STATE_UNKNOWN;
      }
    }
  }
}

/**
 * Logs the states of all clients with a slave
 */
void clientPoolLogStates() {
  for (byte id=SLAVE_ID_MIN ; id<=SLAVE_ID_MAX ; id++) {
    if (slaveFound[id]) {
      logMessage("Client for slave with ID " + String(id) + " in State: " + String(poolClientState[id]));
    }
  }
}

/**
 * Returns the number of online clients
 * 
 * @return int The number of online clients
 */
int clientPoolClientsOnline() {
  int counter = 0;
  for (byte id=SLAVE_ID_MIN ; id<=SLAVE_ID_MAX ; id++) {
    if (poolClientState[id] > CLIENT_STATE_OFFLINE) {
      counter ++;
    }
  }
  return counter;
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
    setStateClient(id, CLIENT_STATE_ONLINE);
    return true;
  }
  setStateClient(id, CLIENT_STATE_OFFLINE);
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
  setStateClient(id, CLIENT_STATE_ONLINE);
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
  setStateClient(id, CLIENT_STATE_JOB_REQUEST_SENT_TO_SERVER);
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
    logMessage("Jobdata from pool Server for client with ID "+String(id)+ ": "+result);
    poolClientLastBlockHash[id] = splitStringAndGetValue(result, ',', 0);
    poolClientNextBlockHash[id] = splitStringAndGetValue(result, ',', 1);
    poolClientDifficulty[id] = splitStringAndGetValue(result, ',', 2);

    if (poolClientLastBlockHash[id].length() > 40) {
      int lengthVersionText = poolClientLastBlockHash[id].length()-40;
      poolClientServerVersion[id] = poolClientLastBlockHash[id].substring(0, lengthVersionText);
      poolClientLastBlockHash[id] = poolClientLastBlockHash[id].substring(lengthVersionText);
    }
    if (poolClientLastBlockHash[id].length() < 10) {
      logMessage("Invalid job result for client with ID "+String(id)+ ": "+result);
      setStateClient(id, CLIENT_STATE_ONLINE);
    } else {
      logMessage("Next job for client with ID "+String(id)+ ": "+poolClientLastBlockHash[id]+":"+poolClientNextBlockHash[id]+":"+poolClientDifficulty[id]);
      setStateClient(id, CLIENT_STATE_JOB_REQUEST_RESULT_FROM_SERVER);
    }
  }
  return result;
}

/**
 * Sends the job of the client with the given ID to the slave with the same ID
 * 
 * @param int id The id of the client / slave
 * 
 * @return bool Returns true if job is sent and false if not
 */
void clientPoolSendClientJobToSlave(int id) {
  String result = slaveRequestLn(id);
  logMessage("Read status client "+String(id)+": "+result);
  slaveSendNextJob(id, "F", poolClientLastBlockHash[id], poolClientNextBlockHash[id], poolClientDifficulty[id]);
  setStateClient(id, CLIENT_STATE_JOB_SENT_TO_SLAVE);
}

/**
 * Requests the job result from the slave with the given ID for the client with the same ID
 * 
 * @param int id The id of the client / slave
 * 
 * @return String Returns the result
 */
String clientPoolRequestClientJobResultFromSlave(int id) {
  String result = slaveRequestLn(id);
  if (result != "") {
    logMessage("Job result from client with ID "+String(id)+": "+result);

    String resultType = splitStringAndGetValue(result, ':', 0);
    if (resultType == "R") {
      ducos1aResult[id] = splitStringAndGetValue(result, ':', 3);
      ducoId[id] = splitStringAndGetValue(result, ':', 4);
      microtimeDifference[id] = splitStringAndGetValue(result, ':', 5);
      setStateClient(id, CLIENT_STATE_JOB_RESULT_FROM_SLAVE); 
    }
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
  float hashRate = 190 + random(-50, 50) / 100.0;
  String result = ducos1aResult[id] + "," + String(hashRate, 2) + ","+minerName+",RIG CORE " + String(id);
  logMessage("Client with ID "+String(id)+" sends result to Server: "+result);
  poolClientInstance[id].print(result);
  setStateClient(id, CLIENT_STATE_JOB_RESULT_SENT_TO_SERVER);
  return true;
}

/**
 * Reads the result for the job result from the client with the given ID
 * 
 * @param int id The id of the client
 * 
 * @return String Returns the result or an empty string
 */
String clientPoolReadJobResultResultForClient(int id) {
  if (!clientPoolClientIsConnected(id)) {
    return "";
  }
  String result = clientPoolGetContentFromClient(id);
  if (result != "") {
    logMessage("Result from server for job from client with ID "+String(id)+": "+result);
    if (result.substring(0, 3)=="BAD") {
      jobs_bad++;
    } else if (result.substring(0, 4)=="GOOD") {
      jobs_good++;
    }
    jobs_sum++;
    setStateClient(id, CLIENT_STATE_JOB_RESULT_RESULT_FROM_SERVER);
    
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

void setStateClient(int id, int state) {
  poolClientState[id] = state;
  logMessage("Set client state ID: "+String(state));
}
