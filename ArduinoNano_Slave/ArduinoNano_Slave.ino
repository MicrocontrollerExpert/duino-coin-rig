/*
 * Project: DuinoCoinRig
 * File:    ArduinoNano_Slave
 * Version: 0.1
 * Purpose: This is the master file that starts and coordinates the slave
 * Author:  Frank Niggemann
 * 
 * Hardware: Arduino Nano
 * 
 * Needed files: ArduinoNano_Helper
 *               ArduinoNano_Iic
 *               ArduinoNano_Log
 *               ArduinoNano_Slave
 * 
 * Needed libraries: ArduinoUniqueID
 *                   DuinoCoin
 *                   StreamString
 *                   Wire
 */

#define PIN_LED_ADDRESS   13
#define PIN_LED_WORKING   13
#define PIN_LED_READY     13
#define PIN_ANALOG_IN     A1

#define SLAVE_STATE_UNKNOWN 0           // The ID for status UNKNOWN (Slave is in an unknown state)
#define SLAVE_STATE_FREE 1              // The ID for status FREE (Slave is free for the next job)
#define SLAVE_STATE_WORKING 2           // The ID for status WORKING (Slave is working on a job)
#define SLAVE_STATE_READY 3             // The ID for status READY (Slave is ready with a job and has a result)
#define SLAVE_STATE_RESULT_READY 4      // The ID for status READY (The result is ready to send)
#define SLAVE_STATE_RESULT_SENT 5       // The ID for status READY (The result has been sent)
#define SLAVE_STATE_ERROR 6             // The ID for status ERROR (Slave has a problem)

int slaveState = SLAVE_STATE_UNKNOWN;

// Methods Helper
int getRandomByte();
String getDucoId();
void ledBlink(int pin, int msOn, int msOff);

// Methods Iic
void iicSetup();
void iicHandlerReceive(int numBytes);
void iicEvaluateBufferReceive();
void iicHandlerRequest();
void iicSetBufferRequestStringEmpty();
void iicSetBufferRequestStringJobResult();

// Methods Log
void logSetup();
void logMessage(String message);
void logMessageSerial(String message);
void logMessageI2C(String message);


void setup() {
  delay(100);
  if (PIN_LED_ADDRESS) {
    pinMode(PIN_LED_ADDRESS, OUTPUT);
    digitalWrite(PIN_LED_ADDRESS, LOW);
  }
  if (PIN_LED_WORKING) {
    pinMode(PIN_LED_WORKING, OUTPUT);
    digitalWrite(PIN_LED_WORKING, LOW);
  }
  if (PIN_LED_READY) {
    pinMode(PIN_LED_READY, OUTPUT);
    digitalWrite(PIN_LED_READY, LOW);
  }
  logSetup();
  delay(getStartupDelay());
  setState(SLAVE_STATE_UNKNOWN);
  int wait = getRandomByte()+getRandomByte()*2+getRandomByte()*3+getRandomByte()*4;
  iicSetup();
}

void loop() {
  if (slaveState == SLAVE_STATE_FREE) {
    iicEvaluateBufferReceive();
  } else if (slaveState == SLAVE_STATE_WORKING) {
    
  } else if (slaveState == SLAVE_STATE_READY) {
    iicSetBufferRequestStringJobResult();
  } else if (slaveState == SLAVE_STATE_RESULT_READY) {
   
  } else if (slaveState == SLAVE_STATE_RESULT_SENT) {
    digitalWrite(PIN_LED_WORKING, LOW);
    digitalWrite(PIN_LED_READY, LOW);
    delay(2000);
    setState(SLAVE_STATE_FREE);
  }
  logMessage("Current state: "+String(slaveState));
  delay(1000);
}

void setState(int state) {
  slaveState = state;
  logMessage("Set state ID: "+String(state));
  if (slaveState == SLAVE_STATE_UNKNOWN) {
    iicSetBufferRequestStringEmpty();
  } else if (slaveState == SLAVE_STATE_FREE) {
    
  } else if (slaveState == SLAVE_STATE_WORKING) {
    
  } else if (slaveState == SLAVE_STATE_READY) {
    
  } else if (slaveState == SLAVE_STATE_RESULT_READY) {
    
  } else if (slaveState == SLAVE_STATE_RESULT_SENT) {
    
  } else if (slaveState == SLAVE_STATE_ERROR) {
    iicSetBufferRequestStringEmpty();
  } else {
    iicSetBufferRequestStringEmpty();
  }
}
