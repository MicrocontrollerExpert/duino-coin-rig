/*
 * Project: DuinoCoinRig
 * File:    ESP8266_ServerHttp
 * Version: 0.1
 * Purpose: The local HTTP server for the user front end
 * Author:  Frank Niggemann
 */



/***********************************************************************************************************************
 * Code ServerHttp
 **********************************************************************************************************************/

/**
 * Initializes the HTTP server part of the software
 */
void serverHttpSetup() {
  logMessage("ServerHttp", "serverHttpSetup", "MethodName", "");
  SPIFFS.begin();

  server.on("/api/status", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send(200, "text/plain", serverHttpApiStatus());
  });

  server.on("/api/log", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send(200, "text/plain", serverHttpApiLog());
  });

  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/index.html");
  });
 
  server.serveStatic("/", SPIFFS, "/").setDefaultFile("index.html");
  
  server.begin();
}

/**
 * Returns the current rig status as JSON
 */
String serverHttpApiStatus() {
  logMessage("ServerHttp", "serverHttpApiStatus", "MethodName", "");
  String apiStatus = "";
  if (nodes_online > 0) {
    apiStatus += "{";
    apiStatus += "\"rig_name\":\""+nameRig+"\",";
    apiStatus += "\"rig_ip\":\""+wifiIp+"\",";
    apiStatus += "\"user_name\":\""+nameUser+"\",";
    apiStatus += "\"pool_address\":\""+serverPoolHost+":"+serverPoolPort+"\",";
    apiStatus += "\"up_and_running_since\":\""+String(workingSeconds)+"\",";
    apiStatus += "\"balance_first_value\":"+String(balanceFirstValue, 10)+",";
    apiStatus += "\"balance_first_timestamp\":"+String(balanceFirstTimestamp)+",";
    apiStatus += "\"nodes\":\""+String(nodes_sum)+"\",";
    apiStatus += "\"nodes_online\":\""+String(nodes_online)+"\",";
    apiStatus += "\"number_of_jobs\":\""+String(jobs_sum)+"\",";
    apiStatus += "\"number_of_blocks\":\""+String(jobs_blocks)+"\",";
    apiStatus += "\"jobs_good\":\""+String(jobs_good)+"\",";
    apiStatus += "\"jobs_bad\":\""+String(jobs_bad)+"\",";
    apiStatus += "\"core_details\":[";

    // ToDo ...
        
    apiStatus += "]";
    apiStatus += "}";
  }
  return apiStatus;
}

/**
 * Returns the log files
 */
String serverHttpApiLog() {
  logMessage("ServerHttp", "serverHttpApiLog", "MethodName", "");
  String content = "Log";
  return content;
}
