/*
 * Project: DuinoCoinRig
 * File:    ArduinoNano_Log
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
void logMessage(String message) {
  if (logSerial) {
    logMessageSerial(message);
  }
}

/**
 * Writes the log message to the serial stream
 */
void logMessageSerial(String message) {
  Serial.println(message);
}
