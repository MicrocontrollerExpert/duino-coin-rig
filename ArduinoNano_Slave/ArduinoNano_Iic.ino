/*
 * Project: DuinoCoinRig
 * File:    ArduinoNano_Iic
 * Version: 0.2
 * Purpose: Communication with the master
 * Author:  Frank Niggemann
 */



/***********************************************************************************************************************
 * Code Iic
 **********************************************************************************************************************/

void iicSetup() {
  pinMode(PIN_IIC_SDA, INPUT_PULLUP);
  pinMode(PIN_IIC_SCL, INPUT_PULLUP);
  
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
  logMessage("Address ID: "+String(iic_id));
  Wire.begin(iic_id);
  Wire.onReceive(iicHandlerReceive);
  Wire.onRequest(iicHandlerRequest);  
  ducoId = getDucoId();
  logMessage("DUCO ID: "+ducoId);
  ledBlink(PIN_LED_ADDRESS, 250, 250);
  ledBlink(PIN_LED_ADDRESS, 250, 250);
  setState(SLAVE_STATE_FREE);
}


void iicHandlerReceive(int numBytes) {
  if (numBytes == 0)
  {
    return;
  }
  while (Wire.available()) {
    char c = Wire.read();
    bufferReceive.write(c);
  }
}

void iicHandlerRequest() {
  char c = '\n';
  if (bufferRequest.available() > 0 && bufferRequest.indexOf('\n') != -1) {
    c = bufferRequest.read();
  }
  Wire.write(c);
}

void iicWorker() {
  if (bufferReceive.available() > 0 && bufferReceive.indexOf('\n') != -1) {

    setState(SLAVE_STATE_WORKING);
    logMessage("Request: "+String(bufferReceive));

    String lastblockhash = bufferReceive.readStringUntil(',');
    String newblockhash = bufferReceive.readStringUntil(',');
    unsigned int difficulty = bufferReceive.readStringUntil('\n').toInt();
    unsigned long startTime = micros();
    unsigned int ducos1result = 0;
    if (difficulty < 655) ducos1result = Ducos1a.work(lastblockhash, newblockhash, difficulty);
    unsigned long endTime = micros();
    unsigned long elapsedTime = endTime - startTime;
    while (bufferRequest.available()) bufferRequest.read();
    bufferRequest.print(String(ducos1result) + "," + String(elapsedTime) + "," + ducoId + "\n");

    setState(SLAVE_STATE_FREE);
    logMessage("Result: "+String(ducos1result) + "," + String(elapsedTime) + "," + ducoId);
  }
}

/*
byte iic_id = 0;
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
      
    } else {
      lastBlockHash = bufferReceive.readStringUntil(',');
      nextBlockHash = bufferReceive.readStringUntil(',');
      unsigned int difficulty = bufferReceive.readStringUntil('\n').toInt();
      logMessage("Receive lastBlockHash: "+lastBlockHash);
      logMessage("Receive nextBlockHash: "+nextBlockHash);
      logMessage("Receive difficulty: "+String(difficulty));
      if (lastBlockHash!="" && nextBlockHash!="") {
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
  String request = String(ducos1aResult)+":"+String(microtimeDifference)+":"+ducoId+"\n";
  logMessage(request);
  bufferRequest.print(request);
  setState(SLAVE_STATE_RESULT_SENT);
}
*/
