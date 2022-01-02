/*
 * Project: DuinoCoinRig
 * File:    ESP8266_Display
 * Version: 0.1
 * Purpose: Control of the display
 * Author:  Frank Niggemann
 */

#define OLED_SDA 4 // D2 - A4 - GPIO4
#define OLED_SCL 5 // D1 - A5 - GPIO5
#define OLED_ADDR 0x3C
#define OLED_FLIP_VERTICALLY 0

#include <Wire.h>
#include "SSD1306Wire.h"

SSD1306Wire display(OLED_ADDR, OLED_SDA, OLED_SCL);

/**
 * Initializes the display part of the software
 */
void displaySetup() {
  display.init();
  if (OLED_FLIP_VERTICALLY) {
    display.flipScreenVertically();
  }
  display.setFont(ArialMT_Plain_10);
  display.resetDisplay();
  display.displayOn();
  display.setContrast(255);
  display.setI2cAutoInit(true);
  displayClear();
  displayScreenHome();
}

/**
 * Clears the LCD display
 */
void displayClear() {
  display.clear();
  display.display();
  delay(10);
}

/**
 * Displays the home screen on the LCD display
 */
void displayScreenHome() {
  display.clear();
  display.setTextAlignment(TEXT_ALIGN_LEFT);
  String text = "";
  if (masterState == MASTER_STATE_BOOTING) {
    text = "Booting";
  } else {
    if (!wifiConnected()) {
      if (!sdCardReady()) {
        text = "No SD card found!";
      } else {
        text = softwareName+" V"+softwareVersion+"\n";
        text+= "Config: "+configStatus+"\n";
        text+= "WiFi: "+wifiGetIp();
      }
    } else {
      text = softwareName+" V"+softwareVersion+"\n";
      text+= "WiFi: "+wifiGetIp()+"\n";
      text+= "Pool: "+clientHttpsGetPoolString()+"\n";
      text+= "Cores: "+String(cores_sum)+" / Online: "+String(cores_online)+"\n";
      text+= "Good: "+String(jobs_good)+" / Bad: "+String(jobs_bad);
    }
  }
  display.drawString(0, 0, text);
  display.display();
  delay(10);
}
