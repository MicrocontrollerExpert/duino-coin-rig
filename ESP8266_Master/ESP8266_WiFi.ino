/*
 * Project: DuinoCoinRig
 * File:    ESP8266_WiFi
 * Version: 0.1
 * Purpose: Communication with the wireless network
 * Author:  Frank Niggemann
 */



/***********************************************************************************************************************
 * Code WiFi
 **********************************************************************************************************************/

/**
 * Initializes the WiFi part of the software
 * 
 * @param String ssid The SSID of the WiFi networt
 * @param String password The password for the WiFi network with the given SSID
 */ 
void wifiSetup(String ssid, String password) {
  logMessage("WiFi", "wifiSetup", "MethodName", "");
  if (ssid!="") {
    setStateMaster(MASTER_STATE_CONNECTING_WIFI);
    logMessage("WiFi", "wifiSetup", "MethodDetail", "Connecting to: " + String(ssid));
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
      delay(200);
    }
    setStateMaster(MASTER_STATE_WIFI_CONNECTED);
    wifiIp = wifiGetIp();
    logMessage("WiFi", "wifiSetup", "MethodDetail", "Connected with IP address: " + wifiIp);
  } else {
    logMessage("WiFi", "wifiSetup", "MethodDetail", "Can't connect to WiFi without SSID");
  }
  displayScreenHome();
}

/**
 * Returns the WiFi connection status
 * 
 * @return bool Returns the connection status 
 */
bool wifiConnected() {
  logMessage("WiFi", "wifiConnected", "MethodName", "");
  if (WiFi.status() != WL_CONNECTED) {
    logMessage("WiFi", "wifiConnected", "MethodDetail", "Return false");
    return false;
  }
  logMessage("WiFi", "wifiConnected", "MethodDetail", "Return true");
  return true;
}

/**
 * Returns the IP of the WiFi connection
 * 
 * @return String Returns the IP of the current WiFi connection or "Not connected"
 */
String wifiGetIp() {
  logMessage("WiFi", "wifiGetIp", "MethodName", "");
  if (!wifiConnected()) {
    logMessage("WiFi", "wifiGetIp", "MethodDetail", "Return 'Not connected'");
    return "Not connected";
  }
  String ip = WiFi.localIP().toString();
  logMessage("WiFi", "wifiGetIp", "MethodDetail", "Return " + ip);
  return ip;
}
