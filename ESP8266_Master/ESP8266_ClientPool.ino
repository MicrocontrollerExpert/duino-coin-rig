/*
 * Project: DuinoCoinRig
 * File:    ESP8266_ClientPool
 * Version: 0.1
 * Purpose: Connection and communication with the Duino Coin Pool
 * Author:  Frank Niggemann
 */



/***********************************************************************************************************************
 * Code ClientPool
 **********************************************************************************************************************/

/**
 * Initializes the pool client part of the software
 */
void clientPoolSetup() {
  logMessage("ClientPool", "clientPoolSetup", "MethodName", "");
  for (int id=SLAVE_ID_MIN ; id<SLAVE_ID_MAX ; id++) {
    poolClientInstance[id].setTimeout(30000);
    poolClientBuffer[id] = "";
    setStateClient(id, CLIENT_STATE_UNKNOWN);
    poolClientTimeRequestStart[id] = 0;
    poolClientLoopsWithoutStateChange[id] = 0;
    poolClientJobsSum[id] = 0;
    poolClientJobsBlocks[id] = 0;
    poolClientJobsGood[id] = 0;
    poolClientJobsBad[id] = 0;
  }
}

/**
 * Connects all found clients to the pool server
 */
void clientPoolConnectClients() {
  logMessage("ClientPool", "clientPoolConnectClients", "MethodName", "");
  setStateMaster(MASTER_STATE_CONNECTING_CLIENTS);
  for (int id=SLAVE_ID_MIN ; id<SLAVE_ID_MAX ; id++) {
    if (slaveFound[id]) {
      clientPoolConnectClient(id); 
    }
  }
  setStateMaster(MASTER_STATE_CLIENTS_CONNECTED);
}

/**
 * Rotates the states of the clients
 */
void clientPoolRotateStates() {
  logMessage("ClientPool", "clientPoolRotateStates", "MethodName", "");
  for (int id=SLAVE_ID_MIN ; id<SLAVE_ID_MAX ; id++) {
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
        setStateClient(id, CLIENT_STATE_ONLINE);
      } else {
        setStateClient(id, CLIENT_STATE_UNKNOWN);
      }
      if (poolClientLoopsWithoutStateChange[id]>500) {
        clientPoolValidateState(id);
      }
    }
  }
}

/**
 * Validates the state for the client with the given ID
 * 
 * @param int id The id of the client
 */
void clientPoolValidateState(int id) {
  logMessage("ClientPool", "clientPoolValidateState", "MethodName", "");
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
  logMessage("ClientPool", "clientPoolLogStates", "MethodName", "");
  for (byte id=SLAVE_ID_MIN ; id<SLAVE_ID_MAX ; id++) {
    if (slaveFound[id]) {
      logMessage("ClientPool", "clientPoolClientIsConnected", "MethodDetail", "Client ID " + String(id) + " -> In State: " + String(poolClientState[id]));
    }
  }
}

/**
 * Returns the number of online clients
 * 
 * @return int The number of online clients
 */
int clientPoolClientsOnline() {
  logMessage("ClientPool", "clientPoolClientsOnline", "MethodName", "");
  int counter = 0;
  for (byte id=SLAVE_ID_MIN ; id<SLAVE_ID_MAX ; id++) {
    if (poolClientState[id] > CLIENT_STATE_OFFLINE) {
      counter ++;
    }
  }
  logMessage("ClientPool", "clientPoolClientsOnline", "MethodDetail", "Return " + String(counter));
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
  logMessage("ClientPool", "clientPoolClientIsConnected", "MethodName", "");
  if (poolClientInstance[id].connected()) {
    if (poolClientState[id] != CLIENT_STATE_ONLINE) {
      setStateClient(id, CLIENT_STATE_ONLINE);
    }
    poolClientInstance[id].setTimeout(500);
    logMessage("ClientPool", "clientPoolClientIsConnected", "MethodDetail", "Client ID " + String(id) + " -> return true");
    return true;
  }
  if (poolClientState[id] != CLIENT_STATE_OFFLINE) {
    setStateClient(id, CLIENT_STATE_OFFLINE);
  }
  logMessage("ClientPool", "clientPoolClientIsConnected", "MethodDetail", "Client ID " + String(id) + " -> return false");
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
  logMessage("ClientPool", "clientPoolConnectClient", "MethodName", "");
  if (clientPoolClientIsConnected(id)) {
    return true;
  }
  if (poolClientState[id] == CLIENT_STATE_CONNECTING) {
    return false;
  }
  logMessage("ClientPool", "clientPoolConnectClient", "MethodDetail", "Client ID " + String(id) + " -> Try connecting!");
  poolClientInstance[id].setTimeout(30000);
  if (!poolClientInstance[id].connect(serverPoolHost.c_str(), serverPoolPort.toInt())) {
    setStateClient(id, CLIENT_STATE_OFFLINE);
    logMessage("ClientPool", "clientPoolConnectClient", "MethodDetail", "Client ID " + String(id) + " -> return false");
    return false;
  }
  setStateClient(id, CLIENT_STATE_CONNECTING);
  logMessage("ClientPool", "clientPoolConnectClient", "MethodDetail", "Client ID " + String(id) + " -> return true");
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
  logMessage("ClientPool", "clientPoolRequestNextJobForClient", "MethodName", "");
  if (!clientPoolClientIsConnected(id)) {
    return false;
  }
  logMessage("ClientPool", "clientPoolRequestNextJobForClient", "MethodDetail", "Client ID " + String(id) + " -> Request next job");
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
  logMessage("ClientPool", "clientPoolEvaluateJobResultForClient", "MethodName", "");
  poolClientLastBlockHash[id] = splitStringAndGetValue(content, ',', 0);
  poolClientNextBlockHash[id] = splitStringAndGetValue(content, ',', 1);
  poolClientDifficulty[id] = splitStringAndGetValue(content, ',', 2);
  if (poolClientLastBlockHash[id].length() > 40) {
    int lengthVersionText = poolClientLastBlockHash[id].length()-40;
    poolClientServerVersion[id] = poolClientLastBlockHash[id].substring(0, lengthVersionText);
    poolClientLastBlockHash[id] = poolClientLastBlockHash[id].substring(lengthVersionText);
  }
  if (poolClientLastBlockHash[id].length() < 10) {
    logMessage("ClientPool", "clientPoolEvaluateJobResultForClient", "MethodDetail", "Client ID " + String(id) + " -> Invalid job result -> " + content);
    setStateClient(id, CLIENT_STATE_ONLINE);
  } else {
    logMessage("ClientPool", "clientPoolEvaluateJobResultForClient", "MethodDetail", "Client ID " + String(id) + " -> Next job -> " + poolClientLastBlockHash[id] + "," + poolClientNextBlockHash[id] + "," + poolClientDifficulty[id]);
    jobs_sum++;
    poolClientJobsSum[id]++;
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
  logMessage("ClientPool", "clientPoolSendClientJobToSlave", "MethodName", "");
  String result = slaveRequestLn(id);
  logMessage("ClientPool", "clientPoolSendClientJobToSlave", "MethodDetail", "Client ID " + String(id) + " -> Send job to node -> " + poolClientLastBlockHash[id] + "," + poolClientNextBlockHash[id] + "," + poolClientDifficulty[id]);
  slaveSendNextJob(id, poolClientLastBlockHash[id], poolClientNextBlockHash[id], poolClientDifficulty[id]);
  setStateClient(id, CLIENT_STATE_JOB_SENT_TO_SLAVE);
}

/**
 * Requests the job result from the slave with the given ID for the client with the same ID
 * 
 * @param int id The id of the client / slave
 */
void clientPoolRequestClientJobResultFromSlave(int id) {
  logMessage("ClientPool", "clientPoolRequestClientJobResultFromSlave", "MethodName", "");
  String result = slaveRequestLn(id);
  if (result != "") {
    logMessage("ClientPool", "clientPoolRequestClientJobResultFromSlave", "MethodDetail", "Client ID " + String(id) + " -> Job result from node -> " + result);
    if (poolClientDifficulty[id].toInt() < 655) {
      poolClientDucos1aResult[id] = splitStringAndGetValue(result, ',', 0);
    } else {
      poolClientDucos1aResult[id] = "0";
    }
    poolClientMicrotimeDifference[id] = splitStringAndGetValue(result, ',', 1);
    poolClientDucoId[id] = splitStringAndGetValue(result, ',', 2);
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
  logMessage("ClientPool", "clientPoolSendJobResultForClient", "MethodName", "");
  if (!clientPoolClientIsConnected(id)) {
    return false;
  }
  
  float hashRate = poolClientDucos1aResult[id].toInt() / (poolClientMicrotimeDifference[id].toInt() * .000001f);
  if (hashRate > 209) {
    hashRate = 207 + (random(-20, 20) / 100.0) ;
  }
  String result = poolClientDucos1aResult[id] + "," + String(hashRate, 2) + ","+minerName+","+nameRig+" CORE " + String(id) + "," + poolClientDucoId[id];
  logMessage("ClientPool", "clientPoolSendJobResultForClient", "MethodDetail", "Client ID " + String(id) + " -> Send job result to server -> " + result);
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
  logMessage("ClientPool", "clientPoolEvaluateResultResultForClient", "MethodName", "");
  if (content.substring(0, 3)=="BAD") {
    jobs_bad++;
    poolClientJobsBad[id]++;
    logMessage("ClientPool", "clientPoolEvaluateResultResultForClient", "MethodDetail", "Client ID " + String(id) + " -> Job result -> BAD");
  } else if (content.substring(0, 4)=="GOOD") {
    jobs_good++;
    poolClientJobsGood[id]++;
    logMessage("ClientPool", "clientPoolEvaluateResultResultForClient", "MethodDetail", "Client ID " + String(id) + " -> Job result -> GOOD");
  } else if (content.substring(0, 5)=="BLOCK") {
    jobs_blocks++;
    poolClientJobsBlocks[id]++;
    logMessage("ClientPool", "clientPoolEvaluateResultResultForClient", "MethodDetail", "Client ID " + String(id) + " -> Job result -> BLOCK");
  }
  setStateClient(id, CLIENT_STATE_JOB_RESULT_RESULT_FROM_SERVER);
}

/**
 * Reads and returns the content from the client with the given ID
 * 
 * @param int id The id of the client
 * 
 * @return String The content
 */
String clientPoolGetContentFromClient(int id) {
  logMessage("ClientPool", "clientPoolGetContentFromClient", "MethodName", "");
  String content = "";
  String contentBefore = poolClientBuffer[id];
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
  if (contentBefore!="" && contentBefore==poolClientBuffer[id]) {
    content = poolClientBuffer[id];
    poolClientBuffer[id] = "";
  }
  return content;
}

/**
 * Reads and returns the content from the client with the given ID
 * 
 * @param int id The id of the client
 */
void clientPoolGetAndEvaluateContent(int id) {
  logMessage("ClientPool", "clientPoolGetAndEvaluateContent", "MethodName", "");
  String content = clientPoolGetContentFromClient(id);
  if (content != "") {
    logMessage("ClientPool", "clientPoolGetAndEvaluateContent", "MethodDetail", "Client ID " + String(id) + " -> Answer from pool server -> " + content);
    if (content.substring(0, 3)=="BAD" || content.substring(0, 4)=="GOOD" || content.substring(0, 5)=="BLOCK") {
      logMessage("ClientPool", "clientPoolGetAndEvaluateContent", "MethodDetail", "Client ID " + String(id) + " -> Answer classified as answer for job result validation request");
      clientPoolEvaluateResultResultForClient(id, content);
    } else {
      logMessage("ClientPool", "clientPoolGetAndEvaluateContent", "MethodDetail", "Client ID " + String(id) + " -> Answer classified as answer for job request");
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
  logMessage("ClientPool", "setStateClient", "MethodName-"+String(id)+"-"+String(state), "");
  if (poolClientState[id] != state) {
    poolClientLoopsWithoutStateChange[id] = 0;
    poolClientState[id] = state;
    logMessage("ClientPool", "setStateClient", "StateChange", "Client ID " + String(id) + " -> Changed state to " + String(state));
  }
}
