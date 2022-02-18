/*
 * Project: DuinoCoinRig
 * File:    ArduinoNano_Helper
 * Version: 0.1
 * Purpose: Contains all helper functions
 * Author:  Frank Niggemann
 */



/***********************************************************************************************************************
 * Code Helper
 **********************************************************************************************************************/

String getDucoId() {
  String ID = "DUCOID"+getPseudoUniqueIdString();
  return ID;
}

String getPseudoUniqueIdString() {
  String result = "";
  byte value[8];
  String hexvalue[16];
  hexvalue[0] = '0';
  hexvalue[1] = '1';
  hexvalue[2] = '2';
  hexvalue[3] = '3';
  hexvalue[4] = '4';
  hexvalue[5] = '5';
  hexvalue[6] = '6';
  hexvalue[7] = '7';
  hexvalue[8] = '8';
  hexvalue[9] = '9';
  hexvalue[10] = 'A';
  hexvalue[11] = 'B';
  hexvalue[12] = 'C';
  hexvalue[13] = 'D';
  hexvalue[14] = 'E';
  hexvalue[15] = 'F';
  for (int i=0 ; i<8 ; i++) {
    value[i] = boot_signature_byte_get(i);
  }
  for (int i=8 ; i<128 ; i++) {
    byte value_before = value[(i%8)];
    value[(i%8)] = (value_before+boot_signature_byte_get(i))%256;
  }
  for (int i=0 ; i<8 ; i++) {
    byte low = value[i] % 16;
    byte high = (value[i]-low) / 16;
    logMessage("low: "+String(low));
    logMessage("high: "+String(high));
    result.concat(hexvalue[high]);  
    result.concat(hexvalue[low]);  
  }
  return result;
}

unsigned long getStartupDelay() {
  byte value[8];
  unsigned long milliseconds = 0;
  for (int i=0 ; i<8 ; i++) {
    value[i] = boot_signature_byte_get(i);
  }
  for (int i=8 ; i<128 ; i++) {
    byte value_before = value[(i%8)];
    value[(i%8)] = (value_before+boot_signature_byte_get(i))%256;
  }
  for (int i=0 ; i<8 ; i++) {
    milliseconds += value[i]*(8^i);
  }
  milliseconds = (milliseconds%1000)*10;
  logMessage("Startupdelay: "+String(milliseconds)+"ms");
  return milliseconds;
}

void ledBlink(int pin, int msOn, int msOff) {
  digitalWrite(pin, HIGH);
  delay(msOn);
  digitalWrite(pin, LOW);
  delay(msOff);
}

void ledSet(bool ledAddress, bool ledFree, bool ledWorking, bool ledReady) {
  if (PIN_LED_ADDRESS) {
    if (ledAddress) {
      digitalWrite(PIN_LED_ADDRESS, HIGH);
      logMessage("PIN_LED_ADDRESS: ON");
    } else {
      digitalWrite(PIN_LED_ADDRESS, LOW);
      logMessage("PIN_LED_ADDRESS: OFF");
    }
  }
  if (PIN_LED_FREE) {
    if (ledFree) {
      digitalWrite(PIN_LED_FREE, HIGH);
      logMessage("PIN_LED_FREE: ON");
    } else {
      digitalWrite(PIN_LED_FREE, LOW);
      logMessage("PIN_LED_FREE: OFF");
    }
  }
  if (PIN_LED_WORKING) {
    if (ledWorking) {
      digitalWrite(PIN_LED_WORKING, HIGH);
      logMessage("PIN_LED_WORKING: ON");
    } else {
      digitalWrite(PIN_LED_WORKING, LOW);
      logMessage("PIN_LED_WORKING: OFF");
    }
  }
  if (PIN_LED_READY) {
    if (ledReady) {
      digitalWrite(PIN_LED_READY, HIGH);
      logMessage("PIN_LED_READY: ON");
    } else {
      digitalWrite(PIN_LED_READY, LOW);
      logMessage("PIN_LED_READY: OFF");
    }
  }
}
