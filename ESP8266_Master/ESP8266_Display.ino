/*
 * Project: DuinoCoinRig
 * File:    ESP8266_Display
 * Version: 0.1
 * Purpose: Control of the display
 * Author:  Frank Niggemann
 */



/***********************************************************************************************************************
 * Code Display
 **********************************************************************************************************************/

/**
 * Initializes the display part of the software
 */
void displaySetup() {
  logMessage("Display", "displaySetup", "MethodName", "");
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
  logMessage("Display", "displayClear", "MethodName", "");
  display.clear();
  display.display();
  delay(10);
}

/**
 * Displays the home screen on the LCD display
 */
void displayScreenHome() {
  logMessage("Display", "displayScreenHome", "MethodName", "");
  display.clear();
  display.setTextAlignment(TEXT_ALIGN_LEFT);
  String text = "";
  if (masterState == MASTER_STATE_BOOTING) {
    text = softwareName+" V"+softwareVersion+"\n";
    text+= "Booting";
  } else {
    if (!wifiConnected()) {
      if (!sdCardReady()) {
        text = softwareName+" V"+softwareVersion+"\n";
        text+= "No SD card found!";
      } else {
        text = softwareName+" V"+softwareVersion+"\n";
        text+= "WiFi: "+wifiGetIp();
      }
    } else {
      text = "WiFi: "+wifiGetIp()+"\n";
      if (serverPoolHost!= "" && serverPoolPort!="") {
        text+= "Pool: "+clientHttpsGetPoolString()+"\n"; 
      } else {
        text+= "Pool: Unknown \n";
      }
      text+= "Nodes: "+String(nodes_sum)+" / Online: "+String(nodes_online)+"\n";
      text+= "Jobs: "+String(jobs_sum)+" / Blocks: "+String(jobs_blocks)+"\n";
      text+= "Good: "+String(jobs_good)+" / Bad: "+String(jobs_bad);
    }
  }
  display.drawString(0, 0, text);
  display.display();
  delay(10);
}
