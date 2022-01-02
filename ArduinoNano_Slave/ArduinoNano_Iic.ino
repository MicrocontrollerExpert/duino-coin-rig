/*
 * Project: DuinoCoinRig
 * File:    ArduinoNano_Iic
 * Version: 0.1
 * Purpose: Communication with the master
 * Author:  Frank Niggemann
 */

#include <Wire.h>
#include <StreamString.h>
#include <DuinoCoin.h>

#define WIRE_SDA 4              // D2 - A4 - GPIO4
#define WIRE_SCL 5              // D1 - A5 - GPIO5
#define WIRE_CLOCK 100000       // The speed

#define IIC_ID_MIN 1            // The first possible address
#define IIC_ID_MAX 127          // The last possible address

#define CHAR_REQUEST_SHORT 'S'  // The byte for the request for a short status
#define CHAR_REQUEST_FULL 'F'   // The byte for the request for a full status

byte iic_id = 0;
char result_type = CHAR_REQUEST_FULL;
StreamString bufferReceive;
String stringReceive = "";
StreamString bufferRequest;
String stringRequest = "";
String lastBlockHash = "";
String nextBlockHash = "";
unsigned int difficulty = 0;
unsigned int ducos1aResult = 0;
unsigned long microtimeStart = 0;
unsigned long microtimeEnd = 0;
unsigned long microtimeDifference = 0;
String ducoId = "";

void iicSetup() {
  Wire.begin();
  for (int id=IIC_ID_MIN; id<IIC_ID_MAX; id++) {
    Wire.beginTransmission(id);
    int result = Wire.endTransmission();
    if (result != 0) {
      iic_id = id;
      break;
    }
  }
  Wire.end();
  logMessage("Use ID: "+String(iic_id));
  Wire.begin(iic_id);
  Wire.onReceive(iicHandlerReceive);
  Wire.onRequest(iicHandlerRequest);  
  ducoId = getDucoId();
  setState(SLAVE_STATE_FREE);
}

void iicHandlerReceive(int howMany) {
  if (howMany == 0)
  {
    return;
  }
  while (Wire.available()) {
    char c = Wire.read();
    bufferReceive.write(c);
  }
}

void iicEvaluateBufferReceive() {
  if (bufferReceive.available() > 0 && bufferReceive.indexOf('\n') != -1) {
    if (bufferReceive.length() < 10) {
      String result_type_string = bufferReceive.readStringUntil('\n');
      result_type = result_type_string.charAt(0);
      logMessage("Receive just result_type: "+String(result_type));
      setState(slaveState);
    } else {
      String result_type_string = bufferReceive.readStringUntil(':');
      result_type = result_type_string.charAt(0);
      lastBlockHash = bufferReceive.readStringUntil(':');
      nextBlockHash = bufferReceive.readStringUntil(':');
      difficulty = bufferReceive.readStringUntil('\n').toInt();
      logMessage("Receive result_type: "+String(result_type));
      logMessage("Receive lastBlockHash: "+lastBlockHash);
      logMessage("Receive nextBlockHash: "+nextBlockHash);
      logMessage("Receive difficulty: "+String(difficulty));
      if (result_type!="" && lastBlockHash!="" && nextBlockHash!="" && difficulty>0) {
        setState(SLAVE_STATE_WORKING);
        microtimeStart = micros();
        ducos1aResult = Ducos1a.work(lastBlockHash, nextBlockHash, difficulty);
        microtimeEnd = micros();
        microtimeDifference = microtimeEnd -
        microtimeStart;
        setState(SLAVE_STATE_READY);
      } else {
        setState(SLAVE_STATE_ERROR);
      }
    }
  }
}

void iicHandlerRequest() {
  char c = '\n';
  if (bufferRequest.available() > 0 && bufferRequest.indexOf('\n') != -1)
  {
    c = bufferRequest.read();
  }
  Wire.write(c);
}

void iicSetBufferRequestStringEmpty() {
  String request = "";
  iicSetBufferRequestString(request);
}

void iicSetBufferRequestStringJobResult() {
  String request = "";
  if (result_type == CHAR_REQUEST_FULL) {
    request = lastBlockHash+":"+nextBlockHash+":"+String(ducos1aResult)+":"+ducoId+":"+String(microtimeDifference);
  }
  iicSetBufferRequestString(request);
}

void iicSetBufferRequestString(String request) {
  while (bufferRequest.available()) bufferRequest.read();
  stringRequest = ""; 
  if (slaveState == SLAVE_STATE_UNKNOWN) {
    stringRequest += PREFIX_UNKNOWN;
  } else if (slaveState == SLAVE_STATE_FREE) {
    stringRequest += PREFIX_FREE;
  } else if (slaveState == SLAVE_STATE_WORKING) {
    stringRequest += PREFIX_WORKING;
  } else if (slaveState == SLAVE_STATE_READY) {
    stringRequest += PREFIX_READY;
  } else if (slaveState == SLAVE_STATE_ERROR) {
    stringRequest += PREFIX_ERROR;
  } else {
    stringRequest += "?";
  }
  stringRequest += ":"+request+"\n";
  logMessage("Request: "+stringRequest);
  bufferRequest.print(stringRequest);
}
