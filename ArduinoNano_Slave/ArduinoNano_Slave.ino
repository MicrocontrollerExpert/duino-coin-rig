/*
 * Project: DuinoCoinRig
 * File:    ArduinoNano_Slave
 * Version: 0.1
 * Purpose: This is the master file that starts and coordinates the slave
 * Author:  Frank Niggemann
 * 
 * This is a modified version of this software: https://github.com/ricaun/DuinoCoinI2C/tree/main/DuinoCoin_Tiny_Slave
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



/***********************************************************************************************************************
 * Includes
 **********************************************************************************************************************/

#include <ArduinoUniqueID.h>
#include <DuinoCoin.h>
#include <StreamString.h>
#include <Wire.h>



/***********************************************************************************************************************
 * Definitions
 **********************************************************************************************************************/

#define PIN_LED_ADDRESS   13
#define PIN_LED_FREE      false
#define PIN_LED_WORKING   13
#define PIN_LED_READY     false

#define SLAVE_STATE_UNKNOWN 0           // The ID for status UNKNOWN (Slave is in an unknown state)
#define SLAVE_STATE_FREE 1              // The ID for status FREE (Slave is free for the next job)
#define SLAVE_STATE_WORKING 2           // The ID for status WORKING (Slave is working on a job)
#define SLAVE_STATE_READY 3             // The ID for status READY (Slave is ready with a job and has a result)
#define SLAVE_STATE_RESULT_READY 4      // The ID for status READY (The result is ready to send)
#define SLAVE_STATE_RESULT_SENT 5       // The ID for status READY (The result has been sent)
#define SLAVE_STATE_ERROR 6             // The ID for status ERROR (Slave has a problem)

#define PIN_IIC_SDA 4                   // D2 - A4 - GPIO4
#define PIN_IIC_SCL 5                   // D1 - A5 - GPIO5
#define IIC_SPEED 100000                // The speed

#define IIC_ID_MIN 1                    // The first possible address
#define IIC_ID_MAX 50                   // The last possible address



/***********************************************************************************************************************
 * Variables
 **********************************************************************************************************************/

byte iic_id = 1;
StreamString bufferReceive;
StreamString bufferRequest;
int slaveState = SLAVE_STATE_UNKNOWN;
bool logSerial = true;
String ducoId = "";



/***********************************************************************************************************************
 * Methods
 **********************************************************************************************************************/

// Methods Helper
String getDucoId();
String getPseudoUniqueIdString();
unsigned long getStartupDelay();
void ledBlink(int pin, int msOn, int msOff);
void ledSet(bool ledAddress, bool ledFree, bool ledWorking, bool ledReady);

// Methods Iic
void iicSetup();
void iicHandlerReceive(int numBytes);
void iicHandlerRequest();
void iicWorker();

// Methods Log
void logSetup();
void logMessage(String message);
void logMessageSerial(String message);



/***********************************************************************************************************************
 * Code Main
 **********************************************************************************************************************/

void setup() {
  delay(100);
  if (PIN_LED_ADDRESS) {
    pinMode(PIN_LED_ADDRESS, OUTPUT);
    digitalWrite(PIN_LED_ADDRESS, LOW);
  }
  if (PIN_LED_FREE) {
    pinMode(PIN_LED_FREE, OUTPUT);
    digitalWrite(PIN_LED_FREE, LOW);
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
  iicSetup();
}

void loop() {
  iicWorker();
  delay(5);
}

void setState(int state) {
  if (slaveState != state) {
    slaveState = state;
    if (slaveState == SLAVE_STATE_UNKNOWN) {
      logMessage("Set state ID: "+String(state)+ " (UNKNOWN)");
      ledSet(false, false, false, false);
    } else if (slaveState == SLAVE_STATE_FREE) {
      logMessage("Set state ID: "+String(state)+ " (FREE)");
      ledSet(false, true, false, false);
    } else if (slaveState == SLAVE_STATE_WORKING) {
      logMessage("Set state ID: "+String(state)+ " (WORKING)");
      ledSet(false, false, true, false);
    } else if (slaveState == SLAVE_STATE_READY) {
      logMessage("Set state ID: "+String(state)+ " (READY)");
      ledSet(false, false, false, true);
    } else if (slaveState == SLAVE_STATE_RESULT_READY) {
      logMessage("Set state ID: "+String(state)+ " (RESULT_READY)");
      ledSet(false, false, false, true);
    } else if (slaveState == SLAVE_STATE_RESULT_SENT) {
      logMessage("Set state ID: "+String(state)+ " (RESULT_SENT)");
      ledSet(false, true, false, false);
    } else if (slaveState == SLAVE_STATE_ERROR) {
      logMessage("Set state ID: "+String(state)+ " (ERROR)");
      ledSet(false, false, false, false);
    } else {
      logMessage("Set state ID: "+String(state)+ " (ERROR)");
      ledSet(false, false, false, false);
    }
  }
}
