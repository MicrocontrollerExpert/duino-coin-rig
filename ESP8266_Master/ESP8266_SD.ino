/*
 * Project: DuinoCoinRig
 * File:    ESP8266_SD
 * Version: 0.1
 * Purpose: Communication with the SD card
 * Author:  Frank Niggemann
 */

#include <SdFat.h>
#define CS_PIN D8

SdFat sd;
SdFile file;
const size_t LINE_DIM = 100;
char line[LINE_DIM];
String config;

/**
 * Initializes the SD part of the software
 */
void sdCardSetup() {
  configStatus = "Check";
}

/**
 * Returns true if the SD card is inserted and ready
 * 
 * @return bool Returns true if the SD card is inserted and ready
 */
bool sdCardReady() {
  if (!sd.begin(CS_PIN, SPI_HALF_SPEED)) {
    configStatus = "SD card error";
    return false;  
  }
  configStatus = "SD card ready";
  return true;
}

/**
 * Returns a describing string for the ready status of the SD card
 * 
 * @return String The describing string for the ready status of the SD card
 */
String sdCardReadyString() {
  if (!sdCardReady()) {
    configStatus = "SD card error";
    return "SD card not ready";  
  }
  configStatus = "SD card ready";
  return "SD card ready";
}

/**
 * Returns true if the config file exists on the SD card
 * 
 * @return bool Returns true if the config file exists on the SD card
 */
bool sdCardConfigFileExists() {
  if (!sdCardReady()) {
    return false; 
  }
  if (!file.open("config.rig", O_READ)) {
    configStatus = "File 'config.rig' missing";
    return false;
  }
  configStatus = "File 'config.rig' found";
  return true;
}

/**
 * Returns a describing string for the existence of the config file on the SD card
 * 
 * @return String The describing string for the existence of the config file on the SD card
 */
String sdCardConfigFileExistsString() {
  if (sdCardConfigFileExists) {
    configStatus = "File 'config.rig' missing";
    return "Config file 'config.rig' found on SD card";  
  }
  return "Config file 'config.rig' not found on SD card";
  configStatus = "File 'config.rig' found";
}

/**
 * Reads the config file and sets the local configuration
 */
void sdCardReadConfigFile() {
  size_t n;
  if (!sdCardReady()) {
    logMessage("No SD card found");
  } else if (!sdCardConfigFileExists()) {
    logMessage("No config file 'config.rig' found on SD card");
  } else {
    setState(MASTER_STATE_LOADING_CONFIG);
    logMessage("Loading config file 'config.rig'");
    while ((n = file.fgets(line, sizeof(line))) > 0) {
      config = String(line);
      config.trim();
      if (config.substring(0, 10) == "wifi_ssid=") {
        wifiSsid = config.substring(10);
        logMessage("Config for WiFi SSID loaded");
      } else if (config.substring(0, 14) == "wifi_password=") {
        wifiPassword = config.substring(14);
        logMessage("Config for WiFi password loaded");
      } else if (config.substring(0, 10) == "name_user=") {
        nameUser = config.substring(10);
        logMessage("Config for user name loaded");
      } else if (config.substring(0, 9) == "name_rig=") {
        nameRig = config.substring(9);
        logMessage("Config for rig name loaded");
      } else if (config.substring(0, 17) == "server_main_host=") {
        serverMainHost = config.substring(17);
        logMessage("Config for main server host loaded");
      } else if (config.substring(0, 17) == "server_main_port=") {
        serverMainPort = config.substring(17);
        logMessage("Config for main server port loaded");
      } else if (config.substring(0, 17) == "server_main_name=") {
        serverMainName = config.substring(17);
        logMessage("Config for main server name loaded");
      } else if (config.substring(0, 17) == "server_pool_host=") {
        serverPoolRequestHost = config.substring(17);
        serverPoolRequestHost = "server.duinocoin.com";
        logMessage("Config for pool request server host loaded");
      } else if (config.substring(0, 17) == "server_pool_port=") {
        serverPoolRequestPort = config.substring(17);
        logMessage("Config for pool request server port loaded");
      } else if (config.substring(0, 17) == "server_pool_name=") {
        serverPoolRequestName = config.substring(17);
        logMessage("Config for pool request server name loaded");
      }
    }
    setState(MASTER_STATE_CONFIG_LOADED);
    logMessage("Config file 'config.rig' loaded");
  }
}
