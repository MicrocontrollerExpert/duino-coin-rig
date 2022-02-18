/*
 * Project: DuinoCoinRig
 * File:    ESP8266_ClientHttps
 * Version: 0.1
 * Purpose: Connection and communication with the Duino Coin Server
 * Author:  Frank Niggemann
 */



/***********************************************************************************************************************
 * Code ClientHttps
 **********************************************************************************************************************/

/**
 * Initializes the HTTPS client part of the software
 */
void clientHttpsSetup() {
  logMessage("Master", "clientHttpsSetup", "MethodName", "");
  https.setTimeout(30000);
}

/**
 * Reads and returns the content from the given URL
 * 
 * @param String url The URL whose content is to be read and returned
 * 
 * @return String The content
 */
String clientHttpsGetContent(String url) {
  logMessage("ClientHttps", "clientHttpsGetContent", "MethodName", "");
  String httpsContent = "";
  std::unique_ptr<BearSSL::WiFiClientSecure>clientSecure(new BearSSL::WiFiClientSecure);
  clientSecure->setInsecure();
  if (https.begin(*clientSecure, url))
  {
    int httpsCode = https.GET();
    if (httpsCode == HTTP_CODE_OK)
    {
      httpsContent = https.getString();
      logMessage("ClientHttps", "clientHttpsGetContent", "MethodDetail", "HTTPS get content: "+httpsContent);
    }
    else
    {
      logMessage("ClientHttps", "clientHttpsGetContent", "MethodDetail", "HTTPS get content failed with errorcode ("+String(httpsCode)+") -> " + https.errorToString(httpsCode));
    }
    https.end();
  }
  return httpsContent;
}

/**
 * Requests a pool configuration from the pool server
 */ 
void clientHttpsRequestPoolConfiguration() {
  logMessage("ClientHttps", "clientHttpsRequestPoolConfiguration", "MethodName", "");
  if (masterState != MASTER_STATE_LOADING_POOL) {
    setStateMaster(MASTER_STATE_LOADING_POOL);
  }
  if (serverPoolHost!="" && serverPoolPort!="") {
    if (masterState != MASTER_STATE_POOL_LOADED) {
      setStateMaster(MASTER_STATE_POOL_LOADED);
    }
    logMessage("ClientHttps", "clientHttpsRequestPoolConfiguration", "MethodDetail", "Updated pool configuration to host " + serverPoolHost + " and port " + serverPoolPort + " with name " + serverPoolName);
    serverPoolError = "";
    return;
  }
  logMessage("ClientHttps", "clientHttpsRequestPoolConfiguration", "MethodDetail", "Request pool from " + urlRequestPool);
  String content = clientHttpsGetContent(urlRequestPool);
  if (content != "") {
    logMessage("ClientHttps", "clientHttpsRequestPoolConfiguration", "MethodDetail", "Return content -> " + content);
    DynamicJsonDocument json(1024);
    deserializeJson(json, content);
    serverPoolHost = String(json["ip"]);
    serverPoolPort = String(json["port"]);
    serverPoolName = String(json["name"]);
    setStateMaster(MASTER_STATE_POOL_LOADED);
    logMessage("ClientHttps", "clientHttpsRequestPoolConfiguration", "MethodDetail", "Updated pool configuration to host " + serverPoolHost + " and port " + serverPoolPort + " with name " + serverPoolName);
    serverPoolError = "";
  } else {
    // ToDo: Create fallback solution!
    logMessage("ClientHttps", "clientHttpsRequestPoolConfiguration", "MethodDetail", "Connection failed! Use fallback configuration!");
  }
}

/**
 * Requests the user balance for the current user and updates the value in the system
 */ 
void clientHttpsRequestUserBalance() {
  logMessage("ClientHttps", "clientHttpsRequestUserBalance", "MethodName", "");
  String content = clientHttpsGetContent(urlRequestUserBalance+nameUser);
  if (content != "") {
    DynamicJsonDocument json(10000);
    deserializeJson(json, content);
    float ducoBalance = json["result"]["balance"];
    if (balanceFirstValue == 0) {
      balanceFirstValue = ducoBalance;
      balanceFirstTimestamp = ntpGetTimestamp();
    }
    balanceLastValue = ducoBalance;
    balanceLastTimestamp = ntpGetTimestamp();
  } else {
    logMessage("ClientHttps", "clientHttpsRequestUserBalance", "MethodDetail", "Connection failed!");
  }
  
}

/**
 * Returns the pool configuration or 'unknown' as result
 * 
 * @return String The pool configuration or 'unknown'
 */
String clientHttpsGetPoolString() {
  logMessage("ClientHttps", "clientHttpsGetPoolString", "MethodName", "");
  String pool = "Unknown";
  if (serverPoolHost!= "" && serverPoolPort!="") {
    pool = serverPoolHost + ":" + serverPoolPort;
  }
  logMessage("ClientHttps", "clientHttpsGetPoolString", "MethodDetail", "Returns " + pool);
  return pool;
}
