/*
 * Project: DuinoCoinRig
 * File:    ESP8266_Master
 * Version: 0.1
 * Purpose: This is the master file that starts and coordinates the services
 * Author:  Frank Niggemann
 * 
 * Hardware: ESP8266
 * 
 * Needed files: ESP8266_ClientHttp
 *               ESP8266_ClientPool
 *               ESP8266_Display
 *               ESP8266_Helper
 *               ESP8266_Log
 *               ESP8266_Master
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

#define MASTER_STATE_UNKNOWN 0              // The ID for status UNKNOWN (Master is in an unknown state)
#define MASTER_STATE_BOOTING 1              // The ID for status BOOTING (System is booting)
#define MASTER_STATE_SCANNING 2             // The ID for status SCANNING (Master scanns for slaves)
#define MASTER_STATE_LOADING_CONFIG 3       // The ID for status LOADING_CONFIG (Master loads configuration)
#define MASTER_STATE_CONFIG_LOADED 4        // The ID for status LOADING_CONFIG (Master loads configuration)
#define MASTER_STATE_CONNECTING_WIFI 5      // The ID for status CONNECTING_WIFI (Master connects to WiFi)
#define MASTER_STATE_WIFI_CONNECTED 6       // The ID for status CONNECTING_WIFI (Master connects to WiFi)
#define MASTER_STATE_LOADING_POOL 7         // The ID for status LOADING_POOL (Master loads pool configuration)
#define MASTER_STATE_POOL_LOADED 8          // The ID for status POOL_LOADED (Master successfully loaded pool configuration)
#define MASTER_STATE_CONNECTING_CLIENTS 9   // The ID for status CONNECTING_CLIENTS (Master connects clients to the pool server)
#define MASTER_STATE_CLIENTS_CONNECTED 10   // The ID for status CLIENTS_CONNECTED (Master connected clients to the pool server)
#define MASTER_STATE_RUNNING 11             // The ID for status RUNNING (Master and slaves are up and running)

#define SLAVE_ID_MIN 1                      // The first possible address for a slave
#define SLAVE_ID_MAX 50                     // The last possible address for a slave

// Configuration
String softwareName             = "DuinoCoinRig";
String softwareVersion          = "0.1";
String minerName                = "AVR I2C v2.7.3";
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
String serverPoolRequestName    = "Pool-Request-Server";      // The name
String serverPoolHost           = "";                         // The host of the pool server
String serverPoolPort           = "";                         // The port to connect to
String serverPoolName           = "Pool-Server";              // The name

// Current state of the rig
int masterState = MASTER_STATE_UNKNOWN;                       // The current state of the master
String serverPoolError          = "";                         // The last pool error
int cores_sum = 0;                                            // The number of connected cores
int cores_online = 0;                                         // The number of cores with a connection to the pool
int jobs_sum = 0;                                             // The number of jobs (complete)
int jobs_blocks = 0;                                          // The number of blocks found
int jobs_good = 0;                                            // The number of good jobs
int jobs_bad = 0;                                             // The number of bad jobs
bool slaveFound[SLAVE_ID_MAX];                                // An array with all possible IDs and the information if there is a slave with this ID

// Methods Master
void setState(int state);

// Methods Client HTTP
void clientHttpSetup();
String clientHttpGetContent(String url);

// Methods Client HTTPS
void clientHttpsSetup();
String clientHttpsGetContent(String url);
void clientHttpsRequestPoolConfiguration();
String clientHttpsGetPoolString();

// Methods Client Pool
void clientPoolSetup();
void clientPoolConnectClients();
void clientPoolRotateStates();
void clientPoolLogStates();
int clientPoolClientsOnline();
bool clientPoolClientIsConnected(int id);
bool clientPoolConnectClient(int id);
bool clientPoolRequestNextJobForClient(int id);
String clientPoolReadJobRequestResultForClient(int id);
void clientPoolSendClientJobToSlave(int id);
void clientPoolRequestClientJobResultFromSlave(int id);
bool clientPoolSendJobResultForClient(int id);
String clientPoolReadJobResultResultForClient(int id);
String clientPoolGetContentFromClient(int id);
void clientPoolGetAndEvaluateContent(int id);
void setStateClient(int id, int state);

// Methods Display
void displaySetup();
void displayClear();
void displayScreenHome();

// Methods Helper
String splitStringAndGetValue(String data, char separator, int index);

// Methods Logging
void logSetup();
void logMessage(String message);
void logMessageSerial(String message);
void logMessageSdCard(String message);

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
  delay(100);
  setState(MASTER_STATE_BOOTING);
  logMessage("");
  logMessage("DuinoCoinRig V0.1");
  displaySetup();
  delay(12000);
  slavesSetup();
  displayScreenHome();
  sdCardSetup();
}

/**
 * The main loop for the software
 */
void loop() {
  displayScreenHome();
  
  if (sdCardReady() && !wifiConnected()) {
    sdCardReadConfigFile();
    displayScreenHome();
  }
  
  if (masterState == MASTER_STATE_CONFIG_LOADED) {
    wifiSetup(wifiSsid, wifiPassword);
    displayScreenHome();
  }
  
  if (masterState == MASTER_STATE_WIFI_CONNECTED) {
    clientHttpsRequestPoolConfiguration();
    displayScreenHome();
  }
  
  if (masterState == MASTER_STATE_POOL_LOADED) {
    clientPoolConnectClients();
    cores_online = clientPoolClientsOnline();
    displayScreenHome();
  }
  
  if (masterState == MASTER_STATE_CLIENTS_CONNECTED) {
    setState(MASTER_STATE_RUNNING);
    displayScreenHome();
  }

  if (masterState == MASTER_STATE_RUNNING) {
    clientPoolRotateStates();
    // clientPoolLogStates();
    displayScreenHome();
  }
  
  delay(50);
}

void setState(int state) {
  masterState = state;
  logMessage("Set state ID: "+String(state));
}
