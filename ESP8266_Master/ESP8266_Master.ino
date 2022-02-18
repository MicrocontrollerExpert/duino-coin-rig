/*
 * Project: DuinoCoinRig
 * File:    ESP8266_Master
 * Version: 0.1
 * Purpose: This is the master file that starts and coordinates the services
 * Author:  Frank Niggemann
 * 
 * Hardware: ESP8266
 * 
 * Needed files: ESP8266_ClientHttps
 *               ESP8266_ClientPool
 *               ESP8266_Display
 *               ESP8266_Helper
 *               ESP8266_Log
 *               ESP8266_Master
 *               ESP8266_NTP
 *               ESP8266_SD
 *               ESP8266_ServerHttp
 *               ESP8266_Slaves
 *               ESP8266_WiFi
 * 
 * Needed libraries: Arduino
 *                   ArduinoJson
 *                   ESP8266HTTPClient
 *                   ESP8266mDNS
 *                   ESP8266WiFi
 *                   ESPAsyncTCP
 *                   ESPAsyncWebServer
 *                   NTPClient
 *                   SdFat
 *                   SPIFFSEditor
 *                   SSD1306Wire
 *                   WiFiClientSecureBearSSL
 *                   WiFiUdp
 *                   Wire
 */



/***********************************************************************************************************************
 * Includes
 **********************************************************************************************************************/

#include <Arduino.h>
#include <ArduinoJson.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266mDNS.h> // OTA libraries
#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <NTPClient.h>
#include <SdFat.h>
#include <SPIFFSEditor.h>
#include <SSD1306Wire.h>
#include <WiFiClientSecureBearSSL.h>
#include <WiFiUdp.h>
#include <Wire.h>



/***********************************************************************************************************************
 * Definitions
 **********************************************************************************************************************/

// Possible states for the master node
#define         MASTER_STATE_UNKNOWN                                  0                                           // The ID for status UNKNOWN (Master is in an unknown state)
#define         MASTER_STATE_BOOTING                                  1                                           // The ID for status BOOTING (System is booting)
#define         MASTER_STATE_SCANNING                                 2                                           // The ID for status SCANNING (Master scanns for slaves)
#define         MASTER_STATE_LOADING_CONFIG                           3                                           // The ID for status LOADING_CONFIG (Master loads configuration)
#define         MASTER_STATE_CONFIG_LOADED                            4                                           // The ID for status LOADING_CONFIG (Master loads configuration)
#define         MASTER_STATE_CONNECTING_WIFI                          5                                           // The ID for status CONNECTING_WIFI (Master connects to WiFi)
#define         MASTER_STATE_WIFI_CONNECTED                           6                                           // The ID for status CONNECTING_WIFI (Master connects to WiFi)
#define         MASTER_STATE_LOADING_POOL                             7                                           // The ID for status LOADING_POOL (Master loads pool configuration)
#define         MASTER_STATE_POOL_LOADED                              8                                           // The ID for status POOL_LOADED (Master successfully loaded pool configuration)
#define         MASTER_STATE_CONNECTING_CLIENTS                       9                                           // The ID for status CONNECTING_CLIENTS (Master connects clients to the pool server)
#define         MASTER_STATE_CLIENTS_CONNECTED                        10                                          // The ID for status CLIENTS_CONNECTED (Master connected clients to the pool server)
#define         MASTER_STATE_RUNNING                                  11                                          // The ID for status RUNNING (Master and slaves are up and running)

// Possible states for the pool server clients
#define         CLIENT_STATE_UNKNOWN                                  0                                           // The ID for status UNKNOWN (Client is in an unknown state)
#define         CLIENT_STATE_OFFLINE                                  1                                           // The ID for status OFFLINE (Client is offline)
#define         CLIENT_STATE_CONNECTING                               2                                           // The ID for status CONNECTING (Client is connecting to pool server)
#define         CLIENT_STATE_ONLINE                                   3                                           // The ID for status ONLINE (Client is connected to pool server)
#define         CLIENT_STATE_JOB_REQUEST_SENT_TO_SERVER               4                                           // The ID for status JOB_REQUEST_SENT_TO_SERVER (Client has sent a job request to the server)
#define         CLIENT_STATE_JOB_REQUEST_RESULT_FROM_SERVER           5                                           // The ID for status JOB_REQUEST_RESULT_FROM_SERVER (Server has answered the request for a job)
#define         CLIENT_STATE_JOB_SENT_TO_SLAVE                        6                                           // The ID for status JOB_SENT_TO_SLAVE (Job has been sent to the slave)
#define         CLIENT_STATE_JOB_RESULT_FROM_SLAVE                    7                                           // The ID for status JOB_RESULT_FROM_SLAVE (Slave has sent the result for the job)
#define         CLIENT_STATE_JOB_RESULT_SENT_TO_SERVER                8                                           // The ID for status JOB_RESULT_SENT_TO_SERVER (Client has sent the job result to the server for verification)
#define         CLIENT_STATE_JOB_RESULT_RESULT_FROM_SERVER            9                                           // The ID for status JOB_RESULT_RESULT_FROM_SERVER (Server has answered the request for verification)
#define         CLIENT_STATE_ERROR                                    10                                          // The ID for status ERROR (The client has an error)

// Slaves
#define         SLAVE_ID_MIN                                          1                                           // The first possible address for a slave
#define         SLAVE_ID_MAX                                          55                                          // The last possible address for a slave
#define         WIRE_PIN_SDA                                          D2                                          // The communication pin for SDA (D2 on ESP8266)
#define         WIRE_PIN_SCL                                          D1                                          // The communication pin for SCL (D1 on ESP8266)
#define         WIRE_CLOCK                                            100000                                      // The I2C communication frequency

// Display
#define         OLED_PIN_SDA                                          D2                                          // The communication pin for SDA (D2 on ESP8266)
#define         OLED_PIN_SCL                                          D1                                          // The communication pin for SCL (D1 on ESP8266)
#define         OLED_ADDR                                             0x3C                                        // The address of the OLED display
#define         OLED_FLIP_VERTICALLY                                  0                                           // With 1 the content of the display will be flipped vertically

// SD
#define         SD_PIN_CS                                             D8                                          // The CS pin (D8 on ESP8266)

/***********************************************************************************************************************
 * Variables
 **********************************************************************************************************************/

// Configuration
String          softwareName                                        = "DuinoCoinRig";                             // The name of this software          
String          softwareVersion                                     = "0.3";                                      // The version of this software
String          minerName                                           = "AVR I2C 3.0";                              // The name of the miner
String          wifiSsid                                            = "";                                         // Your WiFi SSID
String          wifiPassword                                        = "";                                         // Your WiFi password
String          wifiIp                                              = "";                                         // Your WiFi IP
String          nameUser                                            = "";                                         // Your Duino Coin username
String          nameRig                                             = "DuinoCoinRig";                             // Your name for this rig
String          urlRequestPool                                      = "https://server.duinocoin.com/getPool";     // The url to request the pool server
String          urlRequestUserBalance                               = "https://server.duinocoin.com/balances/";   // The url to request the balance
String          serverPoolHost                                      = "192.168.2.16";                                         // The host of the pool server
String          serverPoolPort                                      = "6000";                                         // The port to connect to
String          serverPoolName                                      = "Pool-Server";                              // The name of the pool server
bool            loadConfigFromSdCard                                = true;                                      // With true loads config from SD card and overwrites this config here

// Communication HTTPS
HTTPClient      https;                                                                                            // The used instance of HTTPClient to request content from a HTTPS source (ClientHttps)
String          serverBalanceError                                  = "";                                         // The last balance server error

// Communication pool server
WiFiClient      poolClientInstance[SLAVE_ID_MAX];                                                                 // The used instancec of WiFiClient to communicate with the pool server (ClientPool)
String          poolClientBuffer[SLAVE_ID_MAX];                                                                   // The communication buffer to ensure that only complete datasets will be processed (ClientPool)
String          poolClientServerVersion[SLAVE_ID_MAX];                                                            // The server version for the clients node, reported by the pool server (ClientPool)
String          poolClientLastBlockHash[SLAVE_ID_MAX];                                                            // The last block hash for the clients node, reported by the pool server (ClientPool)
String          poolClientNextBlockHash[SLAVE_ID_MAX];                                                            // The next block hash for the clients node, reported by the pool server (ClientPool)
String          poolClientDifficulty[SLAVE_ID_MAX];                                                               // The difficulty for the clients node, reported by the pool server (ClientPool)
int             poolClientState[SLAVE_ID_MAX];                                                                    // The state of the client (ClientPool)
unsigned long   poolClientTimeRequestStart[SLAVE_ID_MAX];                                                         // The time when the last request was sent to the pool server (ClientPool)
String          poolClientDucos1aResult[SLAVE_ID_MAX];                                                            // The result of the Ducos1a class reported by the slave (ClientPool)
String          poolClientDucoId[SLAVE_ID_MAX];                                                                   // The ID of the slave (ClientPool)
String          poolClientMicrotimeDifference[SLAVE_ID_MAX];                                                      // The time used to calculate the result reported by the slave (ClientPool)
int             poolClientLoopsWithoutStateChange[SLAVE_ID_MAX];                                                  // The number of loops without changing the state (ClientPool)
unsigned long   poolClientJobsSum[SLAVE_ID_MAX];                                                                  // The number of jobs sent to the slave (ClientPool)
unsigned long   poolClientJobsBlocks[SLAVE_ID_MAX];                                                               // The number of found blocks for the slave (ClientPool)
unsigned long   poolClientJobsGood[SLAVE_ID_MAX];                                                                 // The number of correct results for the slave (ClientPool)
unsigned long   poolClientJobsBad[SLAVE_ID_MAX];                                                                  // The number of wrong results for the slave (ClientPool)
String          serverPoolError          = "";                                                                    // The last pool server error

// Display
SSD1306Wire     display(OLED_ADDR, OLED_PIN_SDA, OLED_PIN_SCL);                                                   // The used instance of SSD1306Wire with configuraion for the used display

// Log
bool logSerial                                                      = true;                                       // true -> The log will be sent to the serial communication
bool logClientHttps                                                 = true;                                       // true -> Writes the logs for Part ClientHttps
bool logClientPool                                                  = true;                                       // true -> Writes the logs for Part ClientPool
bool logDisplay                                                     = true;                                       // true -> Writes the logs for Part Display
bool logHelper                                                      = true;                                       // true -> Writes the logs for Part Helper
bool logMaster                                                      = true;                                       // true -> Writes the logs for Part Master
bool logNtp                                                         = true;                                       // true -> Writes the logs for Part Ntp
bool logSdCard                                                      = true;                                       // true -> Writes the logs for Part SdCard
bool logServerHttp                                                  = true;                                       // true -> Writes the logs for Part ServerHttp
bool logSlaves                                                      = true;                                       // true -> Writes the logs for Part Slaves
bool logWiFi                                                        = true;                                       // true -> Writes the logs for Part WiFi
bool logMethodNames                                                 = true;                                       // true -> Writes the logs for method names
bool logMethodDetails                                               = true;                                       // true -> Writes the logs for method details
bool logStateChanges                                                = true;                                       // true -> Writes the logs for state changes 

// NTP
WiFiUDP         ntpUDP;                                                                                           // The used instance of WiFiUDP for UDP communication with NTP server
NTPClient       timeClient(ntpUDP, "pool.ntp.org", 0);                                                            // The used instance of NTPClient with configuration

// SD card
SdFat           sd;                                                                                               // The used instance of SdFat for accessing the SD card
SdFile          file;                                                                                             // The used instance of SdFile for reading a file from SD card
const size_t    LINE_DIM                                            = 100;                                        // The max length of a line in the configuration file
char            line[LINE_DIM];                                                                                   // The array for the lines in the configuration file

// Local HTTP server
AsyncWebServer  server(80);                                                                                       // The used instance of AsyncWebServer for serving a website on port 80
bool            webserverStarted                                    = false;                                      // Set to true once the local web server has started

// Slaves
bool            slaveFound[SLAVE_ID_MAX];                                                                         // An array with all possible IDs and the information if there is a slave with this ID


// Current state of the rig
int             masterState                                         = MASTER_STATE_UNKNOWN;                       // The current state of the master
int             nodes_sum                                           = 0;                                          // The number of connected nodes
int             nodes_online                                        = 0;                                          // The number of nodes with a connection to the pool
unsigned long   jobs_sum                                            = 0;                                          // The number of jobs (complete)
unsigned long   jobs_blocks                                         = 0;                                          // The number of blocks found
unsigned long   jobs_good                                           = 0;                                          // The number of good jobs
unsigned long   jobs_bad                                            = 0;                                          // The number of bad jobs
unsigned long   timestampFirst                                      = 0;                                          // First received timestamp
unsigned long   timestampNow                                        = 0;                                          // Current timestamp
unsigned long   timestampUpdateLast                                 = 0;                                          // Timestamp for last update
unsigned long   timestampUpdateEvery                                = 30000;                                      // Update every 30000ms
unsigned long   workingSeconds                                      = 0;                                          // The number of seconds the rig has been running
unsigned long   balanceFirstTimestamp                               = 0;                                          // Timestamp when the first balance has been received
float           balanceFirstValue                                   = 0;                                          // The first received balance
unsigned long   balanceLastTimestamp                                = 0;                                          // Timestamp when the last balance has been received
float           balanceLastValue                                    = 0;                                          // The last received balance



/***********************************************************************************************************************
 * Methods
 **********************************************************************************************************************/

// Methods Master
void            setStateMaster(int state);

// Methods Client HTTPS
void            clientHttpsSetup();
String          clientHttpsGetContent(String url);
void            clientHttpsRequestPoolConfiguration();
String          clientHttpsGetPoolString();
void            clientHttpsRequestUserBalance();

// Methods Client Pool
void            clientPoolSetup();
void            clientPoolConnectClients();
void            clientPoolRotateStates();
void            clientPoolLogStates();
int             clientPoolClientsOnline();
bool            clientPoolClientIsConnected(int id);
bool            clientPoolConnectClient(int id);
bool            clientPoolRequestNextJobForClient(int id);
String          clientPoolReadJobRequestResultForClient(int id);
void            clientPoolSendClientJobToSlave(int id);
void            clientPoolRequestClientJobResultFromSlave(int id);
bool            clientPoolSendJobResultForClient(int id);
String          clientPoolReadJobResultResultForClient(int id);
String          clientPoolGetContentFromClient(int id);
void            clientPoolGetAndEvaluateContent(int id);
void            setStateClient(int id, int state);

// Methods Display
void            displaySetup();
void            displayClear();
void            displayScreenHome();

// Methods Helper
String          splitStringAndGetValue(String data, char separator, int index);

// Methods Logging
void            logSetup();
void            logMessage(String partName, String methodName, String type, String message);
void            logMessageSerial(String message); 


// Methods NTP
void            ntpSetup();
unsigned long   ntpGetTimestamp();

// Methods SD card
void            sdCardSetup();
bool            sdCardReady();
String          sdCardReadyString();
bool            sdCardConfigFileExists();
String          sdCardConfigFileExistsString();
void            sdCardReadConfigFile();

// Methods HTTP server
void            serverHttpSetup();
String          serverHttpApiStatus();
String          serverHttpApiLog();

// Methods slaves
void            slavesSetup();
void            slavesWireInit();
void            slavesScan();
boolean         slaveExists(byte id);
void            slaveSendMessage(byte id, String message);
void            slavesSendMessage(String message);
void            slaveSendNextJob(byte id, String resultType, String lastBlockHash, String nextBlockHash, String difficulty);
void            slaveSendText(byte id, String text);
void            slaveSendLn(byte id, String text);
String          slaveRequestLn(byte id);

// Methods WiFi
void            wifiSetup(String ssid, String password);
bool            wifiConnected();
String          wifiGetIp();



/***********************************************************************************************************************
 * Code Master
 **********************************************************************************************************************/

/**
 * Initializes the software
 */
void setup() {
  delay(100);
  logSetup();
  delay(100);
  setStateMaster(MASTER_STATE_BOOTING);
  logMessage("Master", "setStateMaster", "MethodName", "\n"+softwareName+" "+softwareVersion+"\n"+"\n");
  displaySetup();
  delay(12000);
  slavesSetup();
  displayScreenHome();
  if (loadConfigFromSdCard) {
    sdCardSetup();
  }
}

/**
 * The main loop for the software
 */
void loop() {
  displayScreenHome();
  
  if (loadConfigFromSdCard && sdCardReady() && !wifiConnected()) {
    sdCardReadConfigFile();
    displayScreenHome();
  } else if (!loadConfigFromSdCard && !wifiConnected()) {
    setStateMaster(MASTER_STATE_CONFIG_LOADED);
    displayScreenHome();
  }
  
  if (masterState == MASTER_STATE_CONFIG_LOADED) {
    wifiSetup(wifiSsid, wifiPassword);
    displayScreenHome();
  }
  
  if (masterState == MASTER_STATE_WIFI_CONNECTED) {
    clientHttpsSetup();
    ntpSetup();
    if (timestampFirst == 0) {
      timestampFirst = ntpGetTimestamp();
      timestampNow = timestampFirst;
      timestampUpdateLast = millis();
    }
    clientHttpsRequestUserBalance();
    clientHttpsRequestPoolConfiguration();
    displayScreenHome();
  }
  
  if (masterState == MASTER_STATE_POOL_LOADED) {
    clientPoolConnectClients();
    nodes_online = clientPoolClientsOnline();
    displayScreenHome();
  }
  
  if (masterState == MASTER_STATE_CLIENTS_CONNECTED) {
    setStateMaster(MASTER_STATE_RUNNING);
    displayScreenHome();
    balanceFirstTimestamp = ntpGetTimestamp();
  }

  if (masterState == MASTER_STATE_RUNNING) {
    clientPoolRotateStates();
    displayScreenHome();
    if (!webserverStarted) {
      serverHttpSetup();
      webserverStarted = true;
    }
  }

  if ((millis() - timestampUpdateLast) > timestampUpdateEvery) {
    timestampNow = ntpGetTimestamp();
    timestampUpdateLast = millis();
    workingSeconds = timestampNow - timestampFirst;
    logMessage("Master", "setStateMaster", "MethodDetail", "workingSeconds = " + String(timestampNow) + " - " + String(timestampFirst) + " = " + String(workingSeconds));
    if (serverPoolHost=="" && serverPoolPort=="" && nodes_online==0) {
      clientHttpsRequestPoolConfiguration();
    }
  } else {
    workingSeconds = timestampNow - timestampFirst + (millis()/1000) - (timestampUpdateLast/1000);
    logMessage("Master", "setStateMaster", "MethodDetail", "workingSeconds = " + String(timestampNow) + " - " + String(timestampFirst) + " + " + String((millis()/1000)) + " - " + String((timestampUpdateLast/1000)) + " = " + String(workingSeconds));
  }
  
  delay(50);
}

/**
 * Sets the master state
 * 
 * @param int setStateMaster The id of the state
 */
void setStateMaster(int state) {
  logMessage("Master", "setStateMaster", "MethodName", "");
  if (masterState != state) {
    masterState = state;
    logMessage("Master", "setStateMaster", "StateChange", "Master changed state to " + String(state));
  }
}
