/*
 * Project: DuinoCoinRig
 * File:    ESP8266_NTP
 * Version: 0.1
 * Purpose: Central NTP management
 * Author:  Frank Niggemann
 */



/***********************************************************************************************************************
 * Code NTP
 **********************************************************************************************************************/

/**
 * Initializes the NTP part of the software
 */
void ntpSetup() {
  logMessage("Ntp", "ntpSetup", "MethodName", "");
  timeClient.begin();
}

/**
 * Returns the current UNIX timestamp
 * 
 * @return unsigned long The current UNIX timestamp
 */
unsigned long ntpGetTimestamp() {
  logMessage("Ntp", "ntpGetTimestamp", "MethodName", "");
  timeClient.update();
  unsigned long timestamp = timeClient.getEpochTime();
  logMessage("Ntp", "ntpGetTimestamp", "MethodDetail", "Return timestamp " + String(timestamp));
  return timestamp;
}
