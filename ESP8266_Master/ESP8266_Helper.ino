/*
 * Project: DuinoCoinRig
 * File:    ESP8266_Helper
 * Version: 0.1
 * Purpose: Contains all helper functions
 * Author:  Frank Niggemann
 */



/***********************************************************************************************************************
 * Code Helper
 **********************************************************************************************************************/

/**
 * Splits a given text by a given separator and returns the part of the text at the given position
 * 
 * @param String text The given text
 * @param char separator The given separator
 * @param int position The position to return
 * 
 * @return String The part of the text at the given position
 */
String splitStringAndGetValue(String text, char separator, int position) {
  logMessage("Helper", "splitStringAndGetValue", "MethodName", "");
  int found = 0;
  int strPosition[] = { 0, -1 };
  int maxPosition = text.length() - 1;

  for (int i = 0; i <= maxPosition && found <= position; i++) {
      if (text.charAt(i) == separator || i == maxPosition) {
          found++;
          strPosition[0] = strPosition[1] + 1;
          strPosition[1] = (i == maxPosition) ? i+1 : i;
      }
  }
  return found > position ? text.substring(strPosition[0], strPosition[1]) : "";
}
