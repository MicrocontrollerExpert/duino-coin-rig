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
unsigned int ducos1aResult = 0;
unsigned long microtimeStart = 0;
unsigned long microtimeEnd = 0;
unsigned long microtimeDifference = 0;
String ducoId = "";

void iicSetup() {
  pinMode(WIRE_SDA, INPUT_PULLUP);
  pinMode(WIRE_SCL, INPUT_PULLUP);
  
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
  ledBlink(PIN_LED_ADDRESS, 250, 250);
  ledBlink(PIN_LED_ADDRESS, 250, 250);
  setState(SLAVE_STATE_FREE);
}

void iicHandlerReceive(int numBytes) {
  if (numBytes != 0) {
    while (Wire.available()) {
      char c = Wire.read();
      bufferReceive.write(c);
      ledBlink(PIN_LED_ADDRESS, 100, 100);
    }
  }
}

void iicEvaluateBufferReceive() {
  if (bufferReceive.available() > 0 && bufferReceive.indexOf('\n') != -1) {
    if (bufferReceive.length() < 10) {
      String result_type_string = bufferReceive.readStringUntil('\n');
      result_type = result_type_string.charAt(0);
      logMessage("Receive just result_type: "+String(result_type));
    } else {
      String message = bufferReceive.readStringUntil(':');
      lastBlockHash = bufferReceive.readStringUntil(':');
      nextBlockHash = bufferReceive.readStringUntil(':');
      unsigned int difficulty = bufferReceive.readStringUntil('\n').toInt();
      logMessage("Receive message: "+String(message));
      logMessage("Receive lastBlockHash: "+lastBlockHash);
      logMessage("Receive nextBlockHash: "+nextBlockHash);
      logMessage("Receive difficulty: "+String(difficulty));
      if (result_type!="" && lastBlockHash!="" && nextBlockHash!="") {
        setState(SLAVE_STATE_WORKING);
        digitalWrite(PIN_LED_WORKING, HIGH);
        microtimeStart = micros();
        ducos1aResult = 0;
        if (difficulty < 655){
          ducos1aResult = Ducos1a.work(lastBlockHash, nextBlockHash, difficulty);
        }
        logMessage("Calculated result: "+String(ducos1aResult));
        microtimeEnd = micros();
        microtimeDifference = microtimeEnd - microtimeStart;
        setState(SLAVE_STATE_READY);
        digitalWrite(PIN_LED_READY, HIGH);
        delay(100);
        iicSetBufferRequestStringJobResult();
      } else {
        setState(SLAVE_STATE_ERROR);
        logMessage("ERROR");
      }
    }
  }
}

void iicHandlerRequest() {
  char c = '\n';
  if (bufferRequest.available() > 0 && bufferRequest.indexOf('\n') != -1) {
    c = bufferRequest.read();
  }
  Wire.write(c);
  if (slaveState == SLAVE_STATE_RESULT_READY && bufferRequest.available() == 0) {
    setState(SLAVE_STATE_RESULT_SENT);
  }
}

void iicSetBufferRequestStringEmpty() {
  String request = "";
}

void iicSetBufferRequestStringJobResult() {
  while (bufferRequest.available()) bufferRequest.read();
  String request = lastBlockHash+":"+nextBlockHash+":"+String(ducos1aResult)+":"+ducoId+":"+String(microtimeDifference)+"\n";
  logMessage(request);
  bufferRequest.print(request);
  setState(SLAVE_STATE_RESULT_SENT);
}
