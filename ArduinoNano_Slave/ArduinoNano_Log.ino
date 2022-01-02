/*
 * Project: DuinoCoinRig
 * File:    ArduinoNano_Log
 * Version: 0.1
 * Purpose: Central log management
 * Author:  Frank Niggemann
 */

bool logSerial = true;
bool logI2C = false;

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
  if (logI2C) {
    logMessageI2C(message);
  }
}

/**
 * Writes the log message to the serial stream
 */
void logMessageSerial(String message) {
  Serial.println(message);
}

/**
 * Writes the log message to the I2C bus
 */
void logMessageI2C(String message) {
  // Maybe later ...
}
