/*
 * Project: DuinoCoinRig
 * File:    ESP32_ClientPool
 * Version: 0.1
 * Purpose: Connection and communication with the Duino Coin Pool
 * Author:  Frank Niggemann
 */

#include <WiFi.h>
#include <WiFiClient.h>

WiFiClient poolClientInstance[SLAVE_ID_MAX];
String poolClientBuffer[SLAVE_ID_MAX];
String poolClientContent[SLAVE_ID_MAX];
String poolClientServerVersion[SLAVE_ID_MAX];
String poolClientLastBlockHash[SLAVE_ID_MAX];
String poolClientNextBlockHash[SLAVE_ID_MAX];
String poolClientDifficulty[SLAVE_ID_MAX];
int poolClientState[SLAVE_ID_MAX];
unsigned long poolClientTimeRequestStart[SLAVE_ID_MAX];

String ducos1aResult[SLAVE_ID_MAX];
String ducoId[SLAVE_ID_MAX];
String microtimeDifference[SLAVE_ID_MAX];

int poolClientLoopsWithoutStateChange[SLAVE_ID_MAX];

#define CLIENT_STATE_UNKNOWN 0
#define CLIENT_STATE_OFFLINE 1
#define CLIENT_STATE_CONNECTING 2
#define CLIENT_STATE_ONLINE 3
#define CLIENT_STATE_JOB_REQUEST_SENT_TO_SERVER 4
#define CLIENT_STATE_JOB_REQUEST_RESULT_FROM_SERVER 5
#define CLIENT_STATE_JOB_SENT_TO_SLAVE 6
#define CLIENT_STATE_JOB_RESULT_FROM_SLAVE 7
#define CLIENT_STATE_JOB_RESULT_SENT_TO_SERVER 8
#define CLIENT_STATE_JOB_RESULT_RESULT_FROM_SERVER 9
#define CLIENT_STATE_ERROR 10

/**
 * Initializes the pool client part of the software
 */
void clientPoolSetup() {
  for (int id=SLAVE_ID_MIN ; id<=SLAVE_ID_MAX ; id++) {
    poolClientInstance[id].setTimeout(30000);
    poolClientBuffer[id] = "";
    setStateClient(id, CLIENT_STATE_UNKNOWN);
    poolClientTimeRequestStart[id] = 0;
    poolClientLoopsWithoutStateChange[id] = 0;
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
      poolClientLoopsWithoutStateChange[id]++;
      if (poolClientState[id] == CLIENT_STATE_UNKNOWN) {
        clientPoolConnectClient(id); 
      } else if (poolClientState[id] == CLIENT_STATE_OFFLINE) {
        clientPoolConnectClient(id); 
      } else if (poolClientState[id] == CLIENT_STATE_CONNECTING) {
        clientPoolClientIsConnected(id);
      } else if (poolClientState[id] == CLIENT_STATE_ONLINE) {
        clientPoolRequestNextJobForClient(id);
      } else if (poolClientState[id] == CLIENT_STATE_JOB_REQUEST_SENT_TO_SERVER) {
        clientPoolGetAndEvaluateContent(id);
      } else if (poolClientState[id] == CLIENT_STATE_JOB_REQUEST_RESULT_FROM_SERVER) {
        clientPoolSendClientJobToSlave(id);
      } else if (poolClientState[id] == CLIENT_STATE_JOB_SENT_TO_SLAVE) {
        clientPoolRequestClientJobResultFromSlave(id);
      } else if (poolClientState[id] == CLIENT_STATE_JOB_RESULT_FROM_SLAVE) {
        clientPoolSendJobResultForClient(id);
      } else if (poolClientState[id] == CLIENT_STATE_JOB_RESULT_SENT_TO_SERVER) {
        clientPoolGetAndEvaluateContent(id);
      } else if (poolClientState[id] == CLIENT_STATE_JOB_RESULT_RESULT_FROM_SERVER) {
        // if ((millis() - poolClientTimeRequestStart[id]) > 5000) {
          poolClientState[id] = CLIENT_STATE_ONLINE;  
        // }
      } else {
        poolClientState[id] = CLIENT_STATE_UNKNOWN;
      }
      if (poolClientLoopsWithoutStateChange[id]>5000) {
        clientPoolValidateState(id);
      }
    }
  }
}

void clientPoolValidateState(int id) {
  if (poolClientState[id] == CLIENT_STATE_UNKNOWN) {
    
  } else if (poolClientState[id] == CLIENT_STATE_OFFLINE) {
    
  } else if (poolClientState[id] == CLIENT_STATE_CONNECTING) {
    
  } else if (poolClientState[id] == CLIENT_STATE_ONLINE) {
    
  } else if (poolClientState[id] == CLIENT_STATE_JOB_REQUEST_SENT_TO_SERVER) {
    setStateClient(id, CLIENT_STATE_ONLINE);
  } else if (poolClientState[id] == CLIENT_STATE_JOB_REQUEST_RESULT_FROM_SERVER) {
    
  } else if (poolClientState[id] == CLIENT_STATE_JOB_SENT_TO_SLAVE) {
    
  } else if (poolClientState[id] == CLIENT_STATE_JOB_RESULT_FROM_SLAVE) {
    
  } else if (poolClientState[id] == CLIENT_STATE_JOB_RESULT_SENT_TO_SERVER) {
    setStateClient(id, CLIENT_STATE_JOB_RESULT_FROM_SLAVE);
  } else if (poolClientState[id] == CLIENT_STATE_JOB_RESULT_RESULT_FROM_SERVER) {
    
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
    poolClientInstance[id].setTimeout(500);
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
  if (poolClientState[id] == CLIENT_STATE_CONNECTING) {
    return false;
  }
  logMessage("Try connecting!");
  poolClientInstance[id].setTimeout(30000);
  if (!poolClientInstance[id].connect(serverPoolHost.c_str(), serverPoolPort)) {
    setStateClient(id, CLIENT_STATE_OFFLINE);
    return false;
  }
  setStateClient(id, CLIENT_STATE_CONNECTING);
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
  poolClientInstance[id].print("JOB," + nameUser + ",AVR\n");
  poolClientTimeRequestStart[id] = millis();
  setStateClient(id, CLIENT_STATE_JOB_REQUEST_SENT_TO_SERVER);
  return true;
}

/**
 * Reads the job result for the client with the given ID
 * 
 * @param int id The id of the client
 * @param String content The content from pool server
 */
void clientPoolEvaluateJobResultForClient(int id, String content) {
  
  poolClientLastBlockHash[id] = splitStringAndGetValue(content, ',', 0);
  poolClientNextBlockHash[id] = splitStringAndGetValue(content, ',', 1);
  poolClientDifficulty[id] = splitStringAndGetValue(content, ',', 2);

  if (poolClientLastBlockHash[id].length() > 40) {
    int lengthVersionText = poolClientLastBlockHash[id].length()-40;
    poolClientServerVersion[id] = poolClientLastBlockHash[id].substring(0, lengthVersionText);
    poolClientLastBlockHash[id] = poolClientLastBlockHash[id].substring(lengthVersionText);
  }
  if (poolClientLastBlockHash[id].length() < 10) {
    logMessage("Invalid job result for client with ID "+String(id)+ ": "+content);
    setStateClient(id, CLIENT_STATE_ONLINE);
  } else {
    logMessage("Next job for client with ID "+String(id)+ ": "+poolClientLastBlockHash[id]+":"+poolClientNextBlockHash[id]+":"+poolClientDifficulty[id]);
    setStateClient(id, CLIENT_STATE_JOB_REQUEST_RESULT_FROM_SERVER);
  }
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
 */
void clientPoolRequestClientJobResultFromSlave(int id) {
  String result = slaveRequestLn(id);
  if (result != "") {
    logMessage("Job result from client with ID "+String(id)+": "+result);

    if (poolClientDifficulty[id].toInt() < 655) {
      ducos1aResult[id] = splitStringAndGetValue(result, ':', 2);
    } else {
      ducos1aResult[id] = "0";
    }
    
    ducoId[id] = splitStringAndGetValue(result, ':', 3);
    microtimeDifference[id] = splitStringAndGetValue(result, ':', 4);
    setStateClient(id, CLIENT_STATE_JOB_RESULT_FROM_SLAVE); 
  }
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
  String result = ducos1aResult[id] + "," + String(hashRate, 2) + ","+minerName+","+nameRig+" CORE " + String(id) + "," + ducoId[id].substring(6,16);
  logMessage("Client with ID "+String(id)+" sends result to Server: "+result);
  poolClientInstance[id].print(result);
  setStateClient(id, CLIENT_STATE_JOB_RESULT_SENT_TO_SERVER);
  return true;
}

/**
 * Evaluates the result for the job result from the client with the given ID
 * 
 * @param int id The client ID
 * @param String content The content from pool server
 * 
 * @return String Returns the result or an empty string
 */
void clientPoolEvaluateResultResultForClient(int id, String content) {
  if (content.substring(0, 3)=="BAD") {
    jobs_bad++;
  } else if (content.substring(0, 4)=="GOOD") {
    jobs_good++;
  } else if (content.substring(0, 5)=="BLOCK") {
    jobs_blocks++;
  }
  jobs_sum++;
  setStateClient(id, CLIENT_STATE_JOB_RESULT_RESULT_FROM_SERVER);
}


void clientPoolReadClient(int id) {
  String contentBefore = poolClientBuffer[id];
  if (poolClientInstance[id].available()) {
    while (poolClientInstance[id].available()) {
      char nextChar = poolClientInstance[id].read();
      if (nextChar != '\n') {
        poolClientBuffer[id] += nextChar;
      } else {
        poolClientContent[id] = poolClientBuffer[id];
        poolClientBuffer[id] = "";
      }
    }
  }
  if (contentBefore!="" && contentBefore==poolClientBuffer[id]) {
    poolClientContent[id] = poolClientBuffer[id];
    poolClientBuffer[id] = "";
  }
}


/**
 * Reads and returns the content from the client with the given ID
 * 
 * @param int id The id of the client
 * 
 * @return String The content
 */
String clientPoolGetContentFromClient(int id) {
  clientPoolReadClient(id);
  String content = "";
  if (poolClientContent[id] != "") {
    content = poolClientContent[id];
    poolClientContent[id] = "";
  }
  return content;
}

/**
 * Reads and returns the content from the client with the given ID
 * 
 * @param int id The id of the client
 */
void clientPoolGetAndEvaluateContent(int id) {
  String content = clientPoolGetContentFromClient(id);
  if (content != "") {
    logMessage("Content from server for client with ID "+String(id)+": "+content);
    if (content.substring(0, 3)=="BAD" || content.substring(0, 4)=="GOOD" || content.substring(0, 5)=="BLOCK") {
      logMessage("Content classified for method clientPoolEvaluateResultResultForClient");
      clientPoolEvaluateResultResultForClient(id, content);
    } else {
      logMessage("Content classified for method clientPoolEvaluateJobResultForClient");
      clientPoolEvaluateJobResultForClient(id, content);
    }
  }
}

/**
 * Sets the state for the client
 *
 * @param int id The client ID 
 * @param int state The new state for the client
 */
void setStateClient(int id, int state) {
  if (poolClientState[id] != state) {
    poolClientLoopsWithoutStateChange[id] = 0;
    poolClientState[id] = state;
    logMessage("Set client state ID: "+String(state));
  }
}
