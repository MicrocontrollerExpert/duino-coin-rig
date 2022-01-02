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

#define PIN_LED_ADDRESS   0
#define PIN_LED_WORKING   LED_BUILTIN
#define PIN_LED_READY     LED_BUILTIN
#define PIN_ANALOG_IN     A1

#define PREFIX_UNKNOWN 'U'      // The prefix for status UNKNOWN (Slave is in an unknown state)
#define PREFIX_FREE 'F'         // The prefix for status FREE (Slave is free for the next job)
#define PREFIX_WORKING 'W'      // The prefix for status WORKING (Slave is working on a job)
#define PREFIX_READY 'R'        // The prefix for status READY (Slave is ready with a job and has a result)
#define PREFIX_ERROR 'E'        // The prefix for status ERROR (Slave has a problem)

#define SLAVE_STATE_UNKNOWN 0   // The ID for status UNKNOWN (Slave is in an unknown state)
#define SLAVE_STATE_FREE 1      // The ID for status FREE (Slave is free for the next job)
#define SLAVE_STATE_WORKING 2   // The ID for status WORKING (Slave is working on a job)
#define SLAVE_STATE_READY 3     // The ID for status READY (Slave is ready with a job and has a result)
#define SLAVE_STATE_ERROR 4     // The ID for status ERROR (Slave has a problem)

int slaveState = SLAVE_STATE_UNKNOWN;

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
  delay(50);
  setState(SLAVE_STATE_UNKNOWN);
  int wait = getRandomByte()+getRandomByte()*2+getRandomByte()*3+getRandomByte()*4;
  iicSetup();
}

void loop() {
  
}

void setState(int state) {
  slaveState = state;
  logMessage("Set state ID: "+String(state));
  if (slaveState == SLAVE_STATE_UNKNOWN) {
    iicSetBufferRequestStringEmpty();
  } else if (slaveState == SLAVE_STATE_FREE) {
    iicSetBufferRequestStringEmpty();
  } else if (slaveState == SLAVE_STATE_WORKING) {
    iicSetBufferRequestStringEmpty();
  } else if (slaveState == SLAVE_STATE_READY) {
    iicSetBufferRequestStringJobResult();
  } else if (slaveState == SLAVE_STATE_ERROR) {
    iicSetBufferRequestStringEmpty();
  } else {
    iicSetBufferRequestStringEmpty();
  }
}
