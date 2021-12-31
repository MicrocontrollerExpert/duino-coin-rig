/*
 * Project: DuinoCoinRig
 * File:    ESP8266_Master
 * Version: 0.1
 * Purpose: This is the master file that starts and coordinates the services
 * Author:  Frank Niggemann
 * 
 * Hardware: ESP8266
 * 
 * Needed files: ESP8266_Master 
 *               ESP8266_ClientHttp
 *               ESP8266_ClientPool
 *               ESP8266_Display
 *               ESP8266_Helper
 *               ESP8266_Log
 *               ESP8266_SD
 *               ESP8266_ServerHttp
 *               ESP8266_Slaves
 *               ESP8266_WiFi
 * 
 * Needed libraries: ArduinoJson
 *                   ESP8266HTTPClient
 *                   ESP8266mDNS
 *                   ESP8266WiFi
 *                   ESPAsyncTCP
 *                   ESPAsyncWebServer
 *                   SdFat
 *                   SPIFFSEditor
 *                   SSD1306Wire
 *                   WiFiClient
 *                   Wire
 */

// Configuration
String softwareName             = "DuinoCoinRig V";
String softwareVersion          = "0.1";
String configStatus             = "";
String wifiSsid                 = "";                         // Your WiFi SSID
String wifiPassword             = "";                         // Your WiFi password
String nameUser                 = "";                         // Your Duino Coin username
String nameRig                  = "";                         // Your name for this rig
String serverMainHost           = "server.duinocoin.com";     // The host of the main server
String serverMainPort           = "2812";                     // The port to connect to
String serverMainName           = "Main-Server";              // The name
String serverPoolRequestHost    = "server.duinocoin.com";     // The host of the pool request server
String serverPoolRequestPort    = "4242";                     // The port to connect to
String serverPoolRequestName    = "Pool-Server";              // The name
String serverPoolHost           = "";                         // The host of the pool server
String serverPoolPort           = "";                         // The port to connect to
String serverPoolName           = "";                         // The name

// Methods Logging
void logSetup();
void logMessage(String message);
void logMessageSerial(String message);
void logMessageSdCard(String message);
void logMessageWebsite(String message);

// Methods Display
void displaySetup();
void displayClear();
void displayScreenHome();

// Methods SD-Card
void sdCardSetup();
bool sdCardReady();
String sdCardReadyString();
bool sdCardConfigFileExists();
String sdCardConfigFileExistsString();
void sdCardReadConfigFile();

// Methods WiFi
void wifiSetup(String ssid, String password);
String wifiGetIp();

/**
 * Initializes the software
 */
void setup() {
  delay(100);
  logSetup();
  logMessage("");
  logMessage("DuinoCoinRig V0.1");
  displaySetup();
  sdCardSetup();
  delay(500);
}

/**
 * The main loop for the software
 */
void loop() {
  // put your main code here, to run repeatedly:
  if (sdCardReady() && !wifiConnected()) {
    sdCardReadConfigFile();
    wifiSetup(wifiSsid, wifiPassword);
  }
  if (serverPoolHost=="" && wifiConnected()) {
    clientHttpRequestPoolConfiguration();
  }
  displayScreenHome();
  delay(1000);
}
