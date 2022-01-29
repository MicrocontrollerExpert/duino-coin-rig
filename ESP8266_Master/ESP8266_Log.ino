/*
 * Project: DuinoCoinRig
 * File:    ESP8266_Log
 * Version: 0.1
 * Purpose: Central log management
 * Author:  Frank Niggemann
 */



/***********************************************************************************************************************
 * Code Log
 **********************************************************************************************************************/

/**
 * Initializes the log part of the software
 */
void logSetup() {
  if (logSerial) {
    Serial.begin(115200);
    Serial.println("\n");
  }
}

/**
 * Writes the message to the configured log destination(s)
 */
void logMessage(String partName, String methodName, String type, String message) {
  bool partOk = false;
  bool typeOk = false;
  
  if (partName=="ClientHttps" && logClientHttps) {
    partOk = true;
  } else if (partName=="ClientPool" && logClientPool) {
    partOk = true;
  } else if (partName=="Display" && logDisplay) {
    partOk = true;
  } else if (partName=="Helper" && logHelper) {
    partOk = true;
  } else if (partName=="Master" && logMaster) {
    partOk = true;
  } else if (partName=="Ntp" && logNtp) {
    partOk = true;
  } else if (partName=="SdCard" && logSdCard) {
    partOk = true;
  } else if (partName=="ServerHttp" && logServerHttp) {
    partOk = true;
  } else if (partName=="Slaves" && logSlaves) {
    partOk = true;
  } else if (partName=="WiFi" && logWiFi) {
    partOk = true;
  }
  
  if (type=="MethodName" && logMethodNames) {
    typeOk = true;
  } else if (type=="MethodDetail" && logMethodDetails) {
    typeOk = true;
  } else if (type=="StateChange" && logStateChanges) {
    typeOk = true;
  }

  if (partOk && typeOk && logSerial) {
    if (message != "") {
      logMessageSerial(partName+"::"+methodName+"() -> "+message);
    } else {
      logMessageSerial(partName+"::"+methodName+"()");
    }
  }
}

/**
 * Writes the log message to the serial stream
 */
void logMessageSerial(String message) {
  Serial.println(message);
}
