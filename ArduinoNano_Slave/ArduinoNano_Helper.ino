/*
 * Project: DuinoCoinRig
 * File:    ArduinoNano_Helper
 * Version: 0.1
 * Purpose: Contains all helper functions
 * Author:  Frank Niggemann
 */

#include <ArduinoUniqueID.h>

int getRandomByte() {
  int result = 0;
  for (int pos=0 ; pos<8 ; pos++) {
    delay(2);
    bitWrite(result, pos, bitRead(analogRead(PIN_ANALOG_IN), 0));
  }
  return result;
}

String getDucoId() {
  String ID = "DUCOID";
  char buff[4];
  for (size_t i = 0; i < 8; i++)
  {
    sprintf(buff, "%02X", (uint8_t) UniqueID8[i]);
    ID += buff;
  }
  return ID;
}
