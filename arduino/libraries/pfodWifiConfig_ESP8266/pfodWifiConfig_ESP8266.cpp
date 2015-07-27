/**
 pfodWifiConfig for Arduino Compatibles
 http://www.forward.com.au/pfod/pfodWifiConfig/index.html

 (c)2015 Forward Computing and Control Pty. Ltd.
 This code may be freely used for both private and commerical use.
 Provide this copyright is maintained.
 */

#include <ESP8266WiFi.h>
#include <EEPROM.h>
#include "pfodWifiConfig.h"
#include "pfodWifiConfig_ESP8266.h"

// if the next line is uncommented you need to call pfodWifiConfig::setDebugStream(&Serial);
#define DEBUG

/**
 * configureAPconfig
 * Sets up temporary Access Point for pfodWifiConfigV1 app to connect to.
 *  Then waits for connection on 10.1.1.1:23 to configure settings
 *
 * It uses 242 bytes of eeprom starting at eepromAddress
 example usage
 #define pfodWifiConfigPASSWORD "CBP1JvFTJVeB7BTMGmVcRb"
 pfodWifiConfig_ESP8266 pfodWifiConfig

 uint16_t ipSources = pfodFeatures::DHCP; // bit or these together pfodFeatures::DHCP|pfodFeatures::STATIC_IP if both are available
 uint16_t security = pfodFeatures::OPEN|pfodFeatures::WPA2; // bit or these together
 
 EEPROM.begin(512);
 pfodWifiConfig.configureAPconfig(eepromAddress, "pfodAPconfigV1", pfodWifiConfigPASSWORD,  23,
 pfodFeatures::SERVER, security, ipSources );
 // configureAPconfig never returns.  Need to reboot afterwards
 */
void pfodWifiConfig_ESP8266::configureAPconfig(int eepromAddress, const char* ssid, const char* password,
    int portNo, byte mode, uint16_t security, uint16_t ipSources) {
  boolean alreadyConnected = false;
  WiFiServer server(portNo);
  WiFiClient client;
  IPAddress local_ip = IPAddress(10,1,1,1);
  IPAddress gateway_ip = IPAddress(10,1,1,1);
  IPAddress subnet_ip = IPAddress(255,255,255,0);

#ifdef DEBUG
  if(debugOut != NULL) {
    debugOut->println(F("configureWifiConnect"));
  }
#endif

  pfodWifiConfigParser parser;
  parser.setDebugStream(debugOut);
  /* Initialise wifi module */
#ifdef DEBUG
  if(debugOut != NULL) {
    debugOut->print(F("\nAttempting to setup Access Point - "));
    debugOut->print(ssid);
    debugOut->print(F("\n with password '"));
    debugOut->print(password);
    debugOut->println('\'');
  }
#endif 
	WiFi.softAP(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
#ifdef DEBUG
  if(debugOut != NULL) {
    debugOut->println(F("."));
  }
#endif
  }
#ifdef DEBUG
  if(debugOut != NULL) {
    debugOut->println();
    debugOut->println(F("Access Point setup"));
  }
#endif
  WiFi.softAPConfig(local_ip, gateway_ip, subnet_ip);

	//Serial.println("done");
	IPAddress myIP = WiFi.softAPIP();
#ifdef DEBUG
  if(debugOut != NULL) {
    debugOut->print(F("AP IP address: "));
    debugOut->println(myIP);
  }
#endif

// Start the server
  server.begin();
  server.setNoDelay(true);
#ifdef DEBUG
    if(debugOut != NULL) {
      debugOut->println(F("Server started"));
    }
#endif
#ifdef DEBUG
    if(debugOut != NULL) {
      debugOut->println(WiFi.localIP());
    }
#endif
  client = server.available(); // evaluates to false if no connection
  /** for testing only
   parser.connect(&Serial); // sets new io stream to read from and write to
   outputFeatures(&parser, mode, security, ipSources);
   while (true) {
   parseInput(&parser, mode, eepromAddress);
   }
   ***/


  while (true) {
    if (!client) { // see if a client is available
      client = server.available(); // evaluates to false if no connection
    } else {
      // have client
      if (!client.connected()) {
        if (alreadyConnected) {
          // client closed so clean up
          alreadyConnected = false;
          if (!client) {
            // nothing to stop
          } else {
            client.stop();
          }
          client = server.available();
        }
      } else {
        // have connected client
        if (!alreadyConnected) {
          parser.connect(&client); // sets new io stream to read from and write to
          outputFeatures(&parser, mode, security, ipSources);
          //parser.println(features);
          alreadyConnected = true;
#ifdef DEBUG
          if(debugOut != NULL) {
            debugOut->println(F("Got connection"));
          }
#endif
        }
        parseInput(&parser, mode, security, ipSources, eepromAddress);
        EEPROM.commit(); // does nothing if nothing to do
      }
    }
  }
}

/**
 *  configureWifiConfig
 *  Joins temporary Access Point setup by pfodWifiConfigV1 app on your mobile
 *  and waits for connection on port 23 to configure settings
 *
 * It uses 242 bytes of eeprom starting at eepromAddress
 example usage
 #define pfodWifiConfigPASSWORD "CBP1JvFTJVeB7BTMGmVcRb"
 pfodWifiConfig_ESP8266 pfodWifiConfig
 
 uint16_t ipSources = pfodFeatures::DHCP; // bit or these together pfodFeatures::DHCP|pfodFeatures::STATIC_IP if both are available
 uint16_t security = pfodFeatures::OPEN|pfodFeatures::WPA2; // bit or these together
 
 EEPROM.begin(512);
 pfodWifiConfig.configureWifiConfig(eepromAddress, "pfodWifiConfigV1", pfodWifiConfigPASSWORD,  23,
 pfodFeatures::SERVER, security, ipSources );
 // configureWifiConfig never returns.  Need to reboot afterwards
 */
void pfodWifiConfig_ESP8266::configureWifiConfig(int eepromAddress, const char* ssid, const char* password,
    int portNo, byte mode, uint16_t security, uint16_t ipSources) {
  boolean alreadyConnected = false;
  WiFiServer server(portNo);
  WiFiClient client;
#ifdef DEBUG
  if(debugOut != NULL) {
    debugOut->println(F("configureWifiConnect"));
  }
#endif

  pfodWifiConfigParser parser;
  parser.setDebugStream(debugOut);
  /* Initialise wifi module */
#ifdef DEBUG
  if(debugOut != NULL) {
    debugOut->print(F("\nAttempting to connect to "));
    debugOut->print(ssid);
    debugOut->print(F("\n with password '"));
    debugOut->print(password);
    debugOut->println('\'');
  }
#endif 
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
#ifdef DEBUG
  if(debugOut != NULL) {
    debugOut->println(F("."));
  }
#endif
  }
#ifdef DEBUG
  if(debugOut != NULL) {
    debugOut->println();
    debugOut->println("WiFi connected");
  }
#endif


// Start the server
  server.begin();
#ifdef DEBUG
    if(debugOut != NULL) {
      debugOut->println(F("Server started"));
    }
#endif
#ifdef DEBUG
    if(debugOut != NULL) {
      debugOut->println(WiFi.localIP());
    }
#endif
  client = server.available(); // evaluates to false if no connection
  /** for testing only
   parser.connect(&Serial); // sets new io stream to read from and write to
   outputFeatures(&parser, mode, security, ipSources);
   while (true) {
   parseInput(&parser, mode, eepromAddress);
   }
   ***/


  while (true) {
    if (!client) { // see if a client is available
      client = server.available(); // evaluates to false if no connection
    } else {
      // have client
      if (!client.connected()) {
        if (alreadyConnected) {
          // client closed so clean up
          alreadyConnected = false;
          if (!client) {
            // nothing to stop
          } else {
            client.stop();
          }
          client = server.available();
        }
      } else {
        // have connected client
        if (!alreadyConnected) {
          parser.connect(&client); // sets new io stream to read from and write to
          outputFeatures(&parser, mode, security, ipSources);
          //parser.println(features);
          alreadyConnected = true;
#ifdef DEBUG
          if(debugOut != NULL) {
            debugOut->println(F("Got connection"));
          }
#endif
        }
        parseInput(&parser, mode, security, ipSources, eepromAddress);
        EEPROM.commit(); // does nothing if nothing to do
      }
    }
  }
}
