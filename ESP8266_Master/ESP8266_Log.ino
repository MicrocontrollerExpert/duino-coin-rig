/*
 * Project: DuinoCoinRig
 * File:    ESP8266_Log
 * Version: 0.1
 * Purpose: Central log management
 * Author:  Frank Niggemann
 */

bool logMessageSerial = true;
bool logMessageSdCard = true;

/**
 * Initializes the log part of the software
 */
void logSetup() {
  Serial.begin(115200);
  Serial.println("\n");
}

/**
 * Writes the message to the configured log destinations
 */
void logMessage(String message) {
  if (logMessageSerial) {
    logMessageSerial(message);
  }
  if (logMessageSdCard) {
    logMessageSdCard(message);
  }
}

/**
 * Writes the log message to the serial stream
 */
void logMessageSerial(String message) {
  Serial.println(message);
}

/**
 * Writes the log message to the SD card
 */
void logMessageSdCard(String message) {
  
}
