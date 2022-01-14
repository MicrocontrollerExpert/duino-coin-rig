/*
 * Project: DuinoCoinRig
 * File:    ESP32_Log
 * Version: 0.1
 * Purpose: Central log management
 * Author:  Frank Niggemann
 */

bool logSerial = true;
bool logSdCard = true;

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
void logMessage(String message) {
  if (logSerial) {
    logMessageSerial(message);
  }
  if (logSdCard) {
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
  // ToDo
}
