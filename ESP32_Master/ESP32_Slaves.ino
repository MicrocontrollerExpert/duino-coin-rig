/*
 * Project: DuinoCoinRig
 * File:    ESP32_Slaves
 * Version: 0.1
 * Purpose: Communication with the slaves
 * Author:  Frank Niggemann
 */

#include <Wire.h>

#define WIRE_SDA 21          // D2 - A4 - GPIO4
#define WIRE_SCL 22          // D1 - A5 - GPIO5
#define WIRE_CLOCK 400000   // The speed

#define PREFIX_UNKNOWN 'U'  // The prefix for status UNKNOWN (Slave is in an unknown state)
#define PREFIX_FREE 'F'     // The prefix for status FREE (Slave is free for the next job)
#define PREFIX_WORKING 'W'  // The prefix for status WORKING (Slave is working on a job)
#define PREFIX_READY 'R'    // The prefix for status READY (Slave is ready with a job and has a result)
#define PREFIX_ERROR 'E'    // The prefix for status ERROR (Slave has a problem)

#define SLAVE_STATE_UNKNOWN 0
#define SLAVE_STATE_FREE 1
#define SLAVE_STATE_WORKING 2
#define SLAVE_STATE_READY 3
#define SLAVE_STATE_ERROR 4

#define CLIENT_STATE_OFFLINE 0
#define CLIENT_STATE_ONLINE 1
#define CLIENT_STATE_JOB_REQUEST_SENT 2
#define CLIENT_STATE_JOB_REQUEST_RESULT 3
#define CLIENT_STATE_JOB_RESULT_SENT 4
#define CLIENT_STATE_JOB_RESULT_RESULT 5

/**
 * Initializes communication with the slaves
 */
void slavesSetup() {
  wireInit();
  slavesScan();
}

/**
 * Initializes the I²C data bus
 */
void wireInit() {
  Wire.begin(WIRE_SDA, WIRE_SCL);
  Wire.setClock(WIRE_CLOCK);
}

/**
 * Searches for existing slaves
 */
void slavesScan() {
  setState(MASTER_STATE_SCANNING);
  logMessage("Start scanning for slaves");
  int counter = 0;
  for (byte idSlave=SLAVE_ID_MIN ; idSlave<SLAVE_ID_MAX ; idSlave++) {
    if (slaveExists(idSlave)) {
      logMessage("Slave found with ID: " + String(idSlave));
      String text = slaveRequestLn(idSlave);
      counter++;
      slaveFound[idSlave] = true;
    } else {
      slaveFound[idSlave] = false;
    }
  }
  cores_sum = counter;
  logMessage("Found "+String(counter)+" slave(s)");
}

/**
 * Checks if the slave with the given ID exists
 * 
 * @param byte id The ID of the slave whose existence is to be checked
 * 
 * @return bool Returns true if slave exists
 */
boolean slaveExists(byte id) {
  wireInit();
  Wire.beginTransmission(id);
  byte result = Wire.endTransmission();
  return (result == 0);
}

/**
 * Sends a message to the slave with the given ID
 * 
 * @param byte id The ID of the slave to which the message is to be sent
 * @param String message The message to be sent to the slave
 */
void slaveSendMessage(byte id, String message) {
  logMessage("Send message to ID: " + id);
  slaveSendLn(id, message);
}

/**
 * Sends a message to all slaves
 * 
 * @param String message The message to be sent to all slaves
 */
void slavesSendMessage(String message) {
  for (byte id=SLAVE_ID_MIN ; id<=SLAVE_ID_MAX ; id++) {
    if (slaveExists(id)) {
      slaveSendText(id, message);
    }
  }
}

/**
 * Sends the next job to the slave with the given ID
 * 
 * @param byte id The ID of the slave to which the text is to be sent
 * @param String hashLastBlock The hash for the last block
 * @param String hashNextBlock The hash for the next block
 * @param int difficulty The difficulty
 */
void slaveSendNextJob(byte id, String resultType, String lastBlockHash, String nextBlockHash, String difficulty) {
  String job = resultType + ":" + lastBlockHash + ":" + nextBlockHash + ":" + difficulty;
  slaveSendLn(id, job);
}

/**
 * Sends a text to the slave with the given ID
 * 
 * @param byte id The ID of the slave to which the text is to be sent
 * @param String text The text to be sent to the slave
 */
void slaveSendText(byte id, String text) {
  logMessage("Send text to slave wit ID "+String(id)+": "+text);
  wireInit();
  for (int pos=0 ; pos<text.length() ; pos++) {
     Wire.beginTransmission(id);
     Wire.write(text.charAt(pos));
     Wire.endTransmission();
  }
}

/**
 * Sends a text to the slave with the given ID and adds a \n to the end of the text
 * 
 * @param byte id The ID of the slave to which the text is to be sent
 * @param String text The text to be sent to the slave
 */
void slaveSendLn(byte id, String text) {
  slaveSendText(id, text + "\n");
}

/**
 * Requests a text line from a slive
 * 
 * @param byte id The ID of the slave from which a line of text is to be requested
 * 
 * @return String Returns the answer line sent by the slave as a result 
 */
String slaveRequestLn(byte id) {
  char end = '\n';
  String text = "";
  boolean done = false;
  wireInit();
  while (!done) {
    Wire.requestFrom(id, 1);
    if (Wire.available()) {
      char c = Wire.read();
      if (c == end) {
        break;
        done = true;
      }
      text += c;
    }
  }
  return text;
}
