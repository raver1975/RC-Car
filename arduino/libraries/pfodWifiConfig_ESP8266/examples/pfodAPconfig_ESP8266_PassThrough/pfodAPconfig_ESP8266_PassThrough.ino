/*
  pfodAPconfig ESP8266 Pass through example
  Load this sketch to the ESP8266 module. Then use pfodWifiConfigV1 to setup your network parameters
  ssid, password and either DHCP or staticIP and portNo.
  Then when you reboot you can connect to the ip:portNo and pass data via the UART in both directions.
  Your Arduino board that is connected to this module should
   delay(1000);
  before calling
   Serial.begin(115200);
  to skip the ESP8266 debug output


 Using pfodWifiConfigV1 to configure for connection to the network
 To start config apply power and then once powered up connect GPIO2 (D2) to GND via 270ohm resistor (within 20sec)
 !!! NOTE: Important!! Do NOT connect GPIO2 to GND while appling power as this prevents the module from starting up
 When config finished remove connection to GPIO2 and turn off and turn on again.

 see http://www.forward.com.au/pfod/pfodWifiConfig/ESP8266/pfodWifiConfig_ESP8266.html for details
 For an example QR code image look in the directory this file is in.
 */

/**
 * pfodWifiConfigV1 for Arduino Compatibles
 * http://www.forward.com.au/pfod/pfodWifiConfig/index.html
 *
 * (c)2015 Forward Computing and Control Pty. Ltd.
 * This code may be freely used for both private and commerical use.
 * Provide this copyright is maintained.
 */
#include <EEPROM.h>
#include <ESP8266WiFi.h>
#include "pfodWifiConfig.h"
#include "pfodWifiConfig_ESP8266.h"

// normally DEBUG is commented out
//#define DEBUG

WiFiServer server(80);
WiFiClient client;

pfodWifiConfig_ESP8266 pfodWifiConfig;

// =============== start of pfodWifiConfigionV1 settings ==============
// update this define with the password from your QR code
// http://www.forward.com.au/pfod/pfodWifiConfig/pfodQRpsk.html
#define pfodWifiConfigPASSWORD "plyWtEDk6uZ0yfmAEM5wMc"
// the ssid is "pfodWifiConfigV1" and the port is 23 -- set by pfodQRpsk program

// note pfodSecurity uses 19 bytes of eeprom usually starting from 0 so
// start the eeprom address from 20 for configureWifiConfig
int eepromAddress = 20;
int wifiSetup_pin = 2; // name the input pin for setup mode detection GPIO2 on most ESP8266 boards
// =============== end of pfodWifiConfigV1 settings ==============

int LED = 0; // set to led output if you have one,  set to -1 if not used
// On ESP8266-01 connect LED + 270ohm resistor from D0 (GPIO0) to +3V3 to indicate when in config mode
// in which case set LED = 0 (default)

void setup() {
  EEPROM.begin(512);
  pinMode(wifiSetup_pin, INPUT_PULLUP);
  Serial.begin(115200);
  delay(10);
  Serial.println();
  for (int i = 20; i > 0; i--) {
#ifdef DEBUG
    Serial.print(i);
    Serial.print(' ');
#endif
    if (digitalRead(wifiSetup_pin) == LOW) {
      break; // continue to config mode
    }
    // else wait for 20sec to let the user press the button
    delay(1000);
  }
#ifdef DEBUG
  Serial.println(F("Starting Setup"));
#endif
  if (LED >= 0) {
    pinMode(LED, OUTPUT); //starts low == off
    digitalWrite(LED, LOW);
  }

  //  pfodWifiConfig.setDebugStream(&Serial); // add this line is using DEBUG in pfodWifiConfig_ESP8266 library code

  //============ pfodWifiConfigV1 config in Access Point mode ====================
  // see if config button is pressed
  pinMode(wifiSetup_pin, INPUT_PULLUP);
  if (digitalRead(wifiSetup_pin) == LOW) {
    if (LED >= 0) {
      digitalWrite(LED, LOW); // show we are in setup mode
    }
#ifdef DEBUG
    Serial.println(F("Setting up Access Point for pfodWifiConfigV1 to connect to"));
#endif
    // connect to temporary wifi network for setup
    // the features determine the format of the {set...} command
    uint16_t ipSources = pfodFeatures::DHCP | pfodFeatures::STATIC_IP; // bit or these together pfodFeatures::DHCP|pfodFeatures::STATIC_IP if both are available
    uint16_t security = pfodFeatures::WPA2; // bit or these together e.g. pfodFeatures::OPEN | pfodFeatures::WPA
    pfodWifiConfig.configureAPconfig(eepromAddress, "pfodAPconfigV1", pfodWifiConfigPASSWORD, 23,
                                       pfodFeatures::SERVER, security, ipSources );
    // configureAPconfig never returns.  Need to reboot afterwards
  }
  //============ end pfodWifiConfigV1 config ====================

  // else button was not pressed continue to load the stored network settings
  if (LED >= 0) {
    digitalWrite(LED, HIGH);
  }
  //else use configured setttings from EEPROM
  // use these local vars
  char ssid[pfodWifiConfig::MAX_SSID_LEN + 1]; // allow for null
  char password[pfodWifiConfig::MAX_PASSWORD_LEN + 1];
  char staticIP[pfodWifiConfig::MAX_STATICIP_LEN + 1];
  uint16_t portNo = 0;
  uint16_t security = 0;
  uint16_t ipSource = 0;
  byte mode = 0;

  pfodWifiConfig.loadNetworkConfigFromEEPROM(eepromAddress, &mode,
      (char*)ssid, pfodWifiConfig::MAX_SSID_LEN + 1, (char*)password,  pfodWifiConfig::MAX_PASSWORD_LEN + 1,
      &security, &portNo, &ipSource, (char*)staticIP,  pfodWifiConfig::MAX_STATICIP_LEN + 1);

  server = WiFiServer(portNo);
  // Initialise wifi module
#ifdef DEBUG
  Serial.println(F("Connecting to AP"));
  Serial.print("ssid '");
  Serial.print(ssid);
  Serial.println("'");
  Serial.print("password '");
  Serial.print(password);
  Serial.println("'");
#endif
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
#ifdef DEBUG
    Serial.print(".");
#endif
  }
#ifdef DEBUG
  Serial.println();
  Serial.println(F("Connected!"));
#endif

  if (*staticIP != '\0') {
    // config static IP
    IPAddress ip(pfodWifiConfig::ipStrToNum(staticIP));
    IPAddress gateway(ip[0], ip[1], ip[2], 1); // set gatway to ... 1
#ifdef DEBUG
    Serial.print(F("Setting gateway to: "));
    Serial.println(gateway);
#endif
    IPAddress subnet(255, 255, 255, 0);
    WiFi.config(ip, gateway, subnet);
  } // else leave as DHCP

  // Start listening for connections
#ifdef DEBUG
  Serial.println(F("Start Server"));
#endif
  server.begin();
  server.setNoDelay(true);
#ifdef DEBUG
  Serial.println(F("Server Started"));
  // Print the IP address
  Serial.print(WiFi.localIP());
  Serial.print(':');
  Serial.println(portNo);
  Serial.println(F("Listening for connections..."));
#endif

  client = server.available();
#ifdef DEBUG
  Serial.print("+++"); // end of setup start listening now
#endif
}

static const size_t bufferSize = 128;
static uint8_t sbuf[bufferSize];


bool alreadyConnected = false;
// the loop routine runs over and over again forever:
void loop() {
  if (!client) { // see if a client is available
    client = server.available(); // evaluates to false if no connection
  } else {
    // have client
    if (!client.connected()) {
      if (alreadyConnected) {
        // client closed so clean up
        closeConnection();
      }
    } else {
      // have connected client
      if (!alreadyConnected) {
        alreadyConnected = true;
      }
    }
  }

  //check UART for data
  if (Serial.available()) {
    size_t len = Serial.available();
    while (len > 0) { // size_t is an unsigned type so >0 is actually redundent
      size_t will_copy = (len < bufferSize) ? len : bufferSize;
      Serial.readBytes(sbuf, will_copy);
      //push UART data to connected client
      if (alreadyConnected) {
        client.write((const uint8_t *)sbuf, will_copy);
        delay(0); // yield
      }
      len -= will_copy;
    }
  }

  if (client) {
    if (client.available()) {
      size_t len = client.available();
      while (len > 0) { // size_t is an unsigned type so >0 is actually redundent
        size_t will_copy = (len < bufferSize) ? len : bufferSize;
        client.readBytes(sbuf, will_copy);
        //push UART data to connected Serial
        Serial.write(sbuf, will_copy);
        delay(0); // yield
        len -= will_copy;
      }
    }
  }
}

void closeConnection() {
  alreadyConnected = false;
  if (!client) {
    return;
  } // else
  client.stop();
  client = server.available(); // evaluates to false if no connection
}