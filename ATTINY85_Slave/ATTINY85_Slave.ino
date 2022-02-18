/*
 * Project: DuinoCoinRig
 * File:    ATTINY85_Slave
 * Version: 0.1
 * Purpose: This is the master file that starts and coordinates the slave
 * Author:  Frank Niggemann
 * 
 * This is a modified version of this software: https://github.com/ricaun/DuinoCoinI2C/tree/main/DuinoCoin_Tiny_Slave
 * 
 * Hardware: ATTINY85
 * 
 * Needed files: ATTINY85_Slave
 *               sha1
 * 
 * Needed libraries: ArduinoUniqueID
 *                   EEPROM
 *                   sha1
 *                   Wire
 */



/***********************************************************************************************************************
 * Includes
 **********************************************************************************************************************/

#include <ArduinoUniqueID.h>
#include <EEPROM.h>
#include "sha1.h"
#include <TinyWire.h>



/***********************************************************************************************************************
 * Definitions
 **********************************************************************************************************************/

// Possible states for the slave node
#define SLAVE_STATE_UNKNOWN                                           0                                           // The ID for status UNKNOWN (Slave is in an unknown state)
#define SLAVE_STATE_FREE                                              1                                           // The ID for status FREE (Slave is free for the next job)
#define SLAVE_STATE_WORKING                                           2                                           // The ID for status WORKING (Slave is working on a job)
#define SLAVE_STATE_READY                                             3                                           // The ID for status READY (Slave is ready with a job and has a result)
#define SLAVE_STATE_RESULT_READY                                      4                                           // The ID for status RESULT READY (The result is ready to send)
#define SLAVE_STATE_RESULT_SENT                                       5                                           // The ID for status RESULT SENT (The result has been sent)
#define SLAVE_STATE_ERROR                                             6                                           // The ID for status ERROR (Slave has a problem)

#define PIN_LED_ADDRESS                                               4                                           // The LED on this pin will blink when the connection had been established
#define PIN_LED_FREE                                                  4                                           // The LED on this pin shows that the node is free
#define PIN_LED_WORKING                                               1                                           // The LED on this pin shows that the node is working
#define PIN_LED_READY                                                 false                                       // The LED on this pin shows that the node is ready with the job

#define PIN_IIC_SDA                                                   0                                           // This is the I²C SDA pin
#define PIN_IIC_SCL                                                   2                                           // This is the I²C SCL pin

#define ADDRESS_I2C                                                   1                                           // Thes I²C ID

#define BUFFER_MAX                                                    88                                          // The size of the main buffer
#define HASH_BUFFER_SIZE                                              20                                          // The size of the hash buffer
#define CHAR_END                                                      '\n'                                        // The char that ends a line
#define CHAR_DOT                                                      ','                                         // The char that separates the datasets



/***********************************************************************************************************************
 * Variables
 **********************************************************************************************************************/

byte iic_id = 1;
int slaveState = SLAVE_STATE_UNKNOWN;
bool logSerial = true;
String ducoId = "";

static const char DUCOID[] PROGMEM = "DUCOID";
static const char ZEROS[] PROGMEM = "000";

static byte address;
static char buffer[BUFFER_MAX];
static uint8_t buffer_position;
static uint8_t buffer_length;
static bool working;
static bool jobdone;

void(* resetFunc) (void) = 0;//declare reset function at address 0



// --------------------------------------------------------------------- //
// setup
// --------------------------------------------------------------------- //

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
  setState(SLAVE_STATE_UNKNOWN);
  initialize_i2c();
}



// --------------------------------------------------------------------- //
// loop
// --------------------------------------------------------------------- //

void loop() {
  do_work();
  millis(); // ????? For some reason need this to work the i2c
}



// --------------------------------------------------------------------- //
// run
// --------------------------------------------------------------------- //

boolean runEvery(unsigned long interval)
{
  static unsigned long previousMillis = 0;
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval)
  {
    previousMillis = currentMillis;
    return true;
  }
  return false;
}



// --------------------------------------------------------------------- //
// work
// --------------------------------------------------------------------- //

void do_work()
{
  if (working)
  {
    do_job();
  }
}

void do_job()
{
  setState(SLAVE_STATE_WORKING);
  unsigned long startTime = millis();
  int job = work();
  unsigned long endTime = millis();
  unsigned int elapsedTime = endTime - startTime;
  memset(buffer, 0, sizeof(buffer));
  char cstr[16];

  // Job
  itoa(job, cstr, 10);
  strcpy(buffer, cstr);
  buffer[strlen(buffer)] = CHAR_DOT;

  // Time
  itoa(elapsedTime, cstr, 10);
  strcpy(buffer + strlen(buffer), cstr);
  strcpy_P(buffer + strlen(buffer), ZEROS);
  buffer[strlen(buffer)] = CHAR_DOT;

  // DUCOID
  strcpy_P(buffer + strlen(buffer), DUCOID);
  for (size_t i = 0; i < 8; i++)
  {
    itoa(UniqueID8[i], cstr, 16);
    if (UniqueID8[i] < 16) strcpy(buffer + strlen(buffer), "0");
    strcpy(buffer + strlen(buffer), cstr);
  }
  
  buffer_position = 0;
  buffer_length = strlen(buffer);
  working = false;
  jobdone = true;
  setState(SLAVE_STATE_FREE);
}

int work()
{
  char delimiters[] = ",";
  char *lastHash = strtok(buffer, delimiters);
  char *newHash = strtok(NULL, delimiters);
  char *diff = strtok(NULL, delimiters);
  buffer_length = 0;
  buffer_position = 0;
  return work(lastHash, newHash, atoi(diff));
}

//#define HTOI(c) ((c<='9')?(c-'0'):((c<='F')?(c-'A'+10):((c<='f')?(c-'a'+10):(0))))
#define HTOI(c) ((c<='9')?(c-'0'):((c<='f')?(c-'a'+10):(0)))
#define TWO_HTOI(h, l) ((HTOI(h) << 4) + HTOI(l))
//byte HTOI(char c) {return ((c<='9')?(c-'0'):((c<='f')?(c-'a'+10):(0)));}
//byte TWO_HTOI(char h, char l) {return ((HTOI(h) << 4) + HTOI(l));}

void HEX_TO_BYTE(char * address, char * hex, int len)
{
  for (int i = 0; i < len; i++) address[i] = TWO_HTOI(hex[2 * i], hex[2 * i + 1]);
}

// DUCO-S1A hasher
uint32_t work(char * lastblockhash, char * newblockhash, int difficulty)
{
  if (difficulty > 655) return 0;
  HEX_TO_BYTE(newblockhash, newblockhash, HASH_BUFFER_SIZE);
  for (int ducos1res = 0; ducos1res < difficulty * 100 + 1; ducos1res++)
  {
    Sha1.init();
    Sha1.print(lastblockhash);
    Sha1.print(ducos1res);
    if (memcmp(Sha1.result(), newblockhash, HASH_BUFFER_SIZE) == 0)
    {
      return ducos1res;
    }
  }
  return 0;
}



// --------------------------------------------------------------------- //
// i2c
// --------------------------------------------------------------------- //

void initialize_i2c(void) {
  pinMode(PIN_IIC_SDA, INPUT_PULLUP);
  pinMode(PIN_IIC_SCL, INPUT_PULLUP);
  TinyWire.begin(ADDRESS_I2C);
  TinyWire.onReceive(iicHandlerReceive);
  TinyWire.onRequest(iicHandlerRequest); 
  ducoId = getDucoId();
  ledBlink(PIN_LED_ADDRESS, 250, 250);
  ledBlink(PIN_LED_ADDRESS, 250, 250);
  setState(SLAVE_STATE_FREE);
}

void iicHandlerReceive(int howMany) {
  if (howMany == 0) return;
  if (working) return;
  if (jobdone) return;

  while (TinyWire.available()) {
    char c = TinyWire.read();
    buffer[buffer_length++] = c;
    if (buffer_length == BUFFER_MAX) buffer_length--;
    if (c == CHAR_END)
    {
      working = true;
    }
  }
}

void iicHandlerRequest() {
  char c = CHAR_END;
  if (jobdone) {
    c = buffer[buffer_position++];
    if ( buffer_position == buffer_length) {
      jobdone = false;
      buffer_position = 0;
      buffer_length = 0;
    }
  }
  TinyWire.write(c);
}

void setState(int state) {
  if (slaveState != state) {
    slaveState = state;
    if (slaveState == SLAVE_STATE_UNKNOWN) {
      ledSet(false, false, false, false);
    } else if (slaveState == SLAVE_STATE_FREE) {
      ledSet(false, true, false, false);
    } else if (slaveState == SLAVE_STATE_WORKING) {
      ledSet(false, false, true, false);
    } else if (slaveState == SLAVE_STATE_READY) {
      ledSet(false, false, false, true);
    } else if (slaveState == SLAVE_STATE_RESULT_READY) {
      ledSet(false, false, false, true);
    } else if (slaveState == SLAVE_STATE_RESULT_SENT) {
      ledSet(false, true, false, false);
    } else if (slaveState == SLAVE_STATE_ERROR) {
      ledSet(false, false, false, false);
    } else {
      ledSet(false, false, false, false);
    }
  }
}



// --------------------------------------------------------------------- //
// DUCO ID
// --------------------------------------------------------------------- //

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
  return milliseconds;
}



// --------------------------------------------------------------------- //
// LED
// --------------------------------------------------------------------- //

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
    } else {
      digitalWrite(PIN_LED_ADDRESS, LOW);
    }
  }
  if (PIN_LED_FREE) {
    if (ledFree) {
      digitalWrite(PIN_LED_FREE, HIGH);
    } else {
      digitalWrite(PIN_LED_FREE, LOW);
    }
  }
  if (PIN_LED_WORKING) {
    if (ledWorking) {
      digitalWrite(PIN_LED_WORKING, HIGH);
    } else {
      digitalWrite(PIN_LED_WORKING, LOW);
    }
  }
  if (PIN_LED_READY) {
    if (ledReady) {
      digitalWrite(PIN_LED_READY, HIGH);
    } else {
      digitalWrite(PIN_LED_READY, LOW);
    }
  }
}
