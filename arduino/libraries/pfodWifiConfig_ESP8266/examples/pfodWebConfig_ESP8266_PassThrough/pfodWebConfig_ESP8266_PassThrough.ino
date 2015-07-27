/*
  pfodWebConfig for ESP8266 Pass 
  Load this sketch to the ESP8266 module. 
  
  To configure the network parameters
  apply power and then once powered up connect GPIO2 (D2) to GND via 270ohm resistor (within 20sec)
 !!! NOTE: Important!! Do NOT connect GPIO2 to GND while appling power as this prevents the module from starting up.
 
 When the board starts up in config mode it creats an Access Point called pfodWebConfig
 with the password set in this code and in the QR code.
 see http://www.forward.com.au/pfod/secureChallengeResponse/keyGenerator/index.html 
 for a secret key generator.
 For an example QR code image look in the directory this file is in.
 
 see http://www.forward.com.au/pfod/pfodWifiConfig/ESP8266/pfodWebConfig_ESP8266.html for details

 Connect your mobile to the pfodWebConfig, scan the QR code with a QR scanner (e.g. QR Droid Private) and copy
 and past the password in the connection on you mobile.
 
 Then open a Web Browser and enter 10.1.1.1 to get the setup page
 Fill in the settings and press Configure to save the setting in the device.
 
 Remove the connection to GPIO2 and reboot to connect to your real network and start a
 server on the port the configured it will then pass data via the UART in both directions

 Your Arduino board that is connected to this module should
   delay(1000);
  before calling
   Serial.begin(9600);
  to skip the ESP8266 debug output

 */

/**
 * pfodWebConfig for Arduino Compatibles
 * http://www.forward.com.au/pfod/pfodWifiConfig/index.html
 *
 * (c)2015 Forward Computing and Control Pty. Ltd.
 * This code may be freely used for both private and commerical use.
 * Provide this copyright is maintained.
 */

#include <EEPROM.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include "pfodWifiConfig.h"
#include "pfodWifiConfig_ESP8266.h"

// normally DEBUG is commented out
//#define DEBUG 

WiFiServer server(80);
WiFiClient client;

pfodWifiConfig_ESP8266 pfodWifiConfig;

// =============== start of pfodWebConfigion settings ==============
// update this define with the password from your QR code
// http://www.forward.com.au/pfod/pfodWifiConfig/pfodQRpsk.html
#define pfodWifiConfigPASSWORD "plyWtEDk6uZ0yfmAEM5wMc"
// the ssid is "pfodWebConfig" and the port is 23 -- set by pfodQRpsk program

// note pfodSecurity uses 19 bytes of eeprom usually starting from 0 so
// start the eeprom address from 20 for configureWifiConfig
int eepromAddress = 20;
int wifiSetup_pin = 2; // name the input pin for setup mode detection GPIO2 on most ESP8266 boards
// =============== end of pfodWifiConfigV1 settings ==============

int LED = 5; // set to led output if you have one,  set to -1 if not used
// On ESP8266-01 connect LED + 270ohm resistor from D0 (GPIO0) to +3V3 to indicate when in config mode
// in which case set LED = 0 (default)

char ssid[pfodWifiConfig::MAX_SSID_LEN + 1]; // allow for null  field 1
char security[pfodWifiConfig::MAX_SECURITY_LEN + 1]; // field 2
char password[pfodWifiConfig::MAX_PASSWORD_LEN + 1]; // field 3
char staticIP[pfodWifiConfig::MAX_STATICIP_LEN + 1]; // field 5 use dhcp if empty
char hostName[pfodWifiConfig::MAX_HOSTNAME_LEN + 1]; // field 6
char userName[pfodWifiConfig::MAX_USERNAME_LEN + 1]; // field 7
char userPw[pfodWifiConfig::MAX_USER_PASSWORD_LEN + 1]; // field 8


uint16_t ipSources = pfodFeatures::DHCP | pfodFeatures::STATIC_IP; // bit or these together pfodFeatures::DHCP|pfodFeatures::STATIC_IP if both are available
uint16_t securityFeatures = pfodFeatures::WPA | pfodFeatures::WPA2; // bit or these together e.g. pfodFeatures::OPEN | pfodFeatures::WPA

ESP8266WebServer webserver ( 80 );

String msg;
byte inConfigMode = 0; // false

void setup ( void ) {

  WiFi.mode(WIFI_STA);
  inConfigMode = 0; // non in config mode
  EEPROM.begin(512);
  pinMode(wifiSetup_pin, INPUT_PULLUP);
  Serial.begin (9600);
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
  Serial.println();
  Serial.println(F("Starting Setup"));
#endif
  if (LED >= 0) {
    pinMode(LED, OUTPUT); //starts low == off
    digitalWrite(LED, LOW);
  }
  //  pfodWifiConfig.setDebugStream(&Serial); // add this line is using DEBUG in pfodWifiConfig_ESP8266 library code

  //============ pfodWebConfig config in Access Point mode ====================
  // see if config button is pressed
  pinMode(wifiSetup_pin, INPUT_PULLUP);
  if (digitalRead(wifiSetup_pin) == LOW) {
    if (LED >= 0) {
      digitalWrite(LED, HIGH); // show we are in setup mode
    }
    inConfigMode = 1; // in config mode
    WiFi.mode(WIFI_AP_STA);

#ifdef DEBUG
    Serial.println(F("Setting up Access Point for pfodWebConfig to connect to"));
#endif
    // connect to temporary wifi network for setup
    // the features determine the format of the {set...} command
    setupAP("pfodWebConfig", pfodWifiConfigPASSWORD);
    //   Need to reboot afterwards
    return; // skip rest of setup();
  }
  //============ end pfodWebConfig config ====================

  // else button was not pressed continue to load the stored network settings
  if (LED >= 0) {
    digitalWrite(LED, LOW);
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

void setupAP(const char* ssid_wifi, const char* password_wifi) {
  const char *aps = scanForAPs();
  delay(0);
#ifdef DEBUG
  Serial.println(aps);
#endif

  IPAddress local_ip = IPAddress(10,1,1,1);
  IPAddress gateway_ip = IPAddress(10,1,1,1);
  IPAddress subnet_ip = IPAddress(255,255,255,0);

#ifdef DEBUG
    Serial.println(F("configure pfodWebConnect"));
#endif

WiFi.softAP(ssid_wifi, password_wifi);
  
#ifdef DEBUG
    Serial.println();
    Serial.println(F("Access Point setup"));
#endif
  WiFi.softAPConfig(local_ip, gateway_ip, subnet_ip);

#ifdef DEBUG
  Serial.println("done");
  IPAddress myIP = WiFi.softAPIP();
    Serial.print(F("AP IP address: "));
    Serial.println(myIP);
#endif


  msg = "<html>"
        "<head>"
        "<title>Server Setup</title>"
        "<meta charset=\"utf-8\" />"
        "<meta name=viewport content=\"width=device-width, initial-scale=1\">"
        "</head>"
        "<body>"
        "<h2>Server Setup</h2>"
        "<p>Use this form to configure your device to connect to your Wifi network and start as a Server listening on the specified port.</p>"
        "<form class=\"form\" method=\"post\" action=\"/config\" >"
        "<p class=\"name\">"
        "<label for=\"name\">Network SSID</label><br>"
        "(Spaces and '+' characters are NOT allowed)<br>"
        "<input type=\"text\" name=\"1\" id=\"ssid\" placeholder=\"wifi network name\"  required " // field 1
        " pattern=\"\\s*[^(\\+|\\s)]+\\s*\" ";

  if (*aps != '\0') {
    msg += " value=\"";
    msg += aps;
    msg += "\" ";
  }
  msg += " />"
         "</p> "
         "<p class=\"security\">"
         "<label for=\"security\">Security</label><br>"
         "<select name=\"2\" id=\"security\" required>" // field 2
         "<option value=\"WPA/WPA2\">WPA/WPA2</option>"
         "</select>"
         "</p>"
         "<p class=\"password\">"
         "<label for=\"password\">Password</label><br>"
         "(Spaces and '+' characters are NOT not allowed)<br>"
         "<input type=\"text\" name=\"3\" id=\"password\" placeholder=\"wifi network password\" autocomplete=\"off\" required " // field 3
         " pattern=\"\\s*[^(\\+|\\s)]+\\s*\"/>"
         "</p>"
         "<p class=\"static_ip\">"
         "<label for=\"static_ip\">Set the Static IP for this device</label><br>"
         "(If this field is empty, DHCP will be used to get an IP address)<br>"
         "<input type=\"text\" name=\"5\" id=\"static_ip\" placeholder=\"192.168.4.99\" "  // field 5
         " pattern=\"\\b(?:(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\\.){3}(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\\b\"/>"
         "</p>"
         "<p class=\"portNo\">"
         "<label for=\"portNo\">Set the port number that the Server will listen on for connections.</label><br>"
         "<input type=\"text\" name=\"4\" id=\"portNo\" placeholder=\"80\" required"  // field 4
         " pattern=\"\\b([0-9]{1,4}|[1-5][0-9]{4}|6[0-4][0-9]{3}|65[0-4][0-9]{2}|655[0-2][0-9]|6553[0-5])\\b\" />"
         "</p>"
         "<p class=\"submit\">"
         "<input type=\"submit\" value=\"Configure\"  />"
         "</p>"
         "</form>"
         "</body>"
         "</html>";


  delay(100);

  webserver.on ( "/", handleRoot );
  webserver.on ( "/config", handleConfig );
  webserver.onNotFound ( handleNotFound );
  webserver.begin();
#ifdef DEBUG
  Serial.println ( "HTTP webserver started" );
#endif
}

static const size_t bufferSize = 128;
static uint8_t sbuf[bufferSize];

bool alreadyConnected = false;
// the loop routine runs over and over again forever:
void loop() {
  if (inConfigMode) {
    webserver.handleClient();
    delay(0);
    return;
  }

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

const size_t MAX_SSID_LEN = 32;
char ssid_found[MAX_SSID_LEN + 1]; // max 32 chars + null

// will always put '\0\ at dest[maxLen]
// return the number of char copied excluding the terminating null
size_t strncpy_safe(char* dest, const char* src, size_t maxLen) {
  size_t rtn = 0;
  if (src == NULL) {
    dest[0] = '\0';
  } else {
    strncpy(dest, src, maxLen);
    rtn = strlen(src);
    if ( rtn > maxLen) {
      rtn = maxLen;
    }
  }
  dest[maxLen] = '\0';
  return rtn;
}

const char* scanForAPs() {
  // WiFi.scanNetworks will return the number of networks found
  int8_t n = WiFi.scanNetworks();
#ifdef DEBUG
  Serial.print ("Scan done\n");
#endif
  delay(0);
  int32_t maxRSSI = -1000;
  strncpy_safe((char*)ssid_found, "", MAX_SSID_LEN); // empty
  if (n <= 0) {
#ifdef DEBUG
    Serial.print("No networks found\n");
#endif
  } else {
#ifdef DEBUG
    Serial.print("Networks found:");
    Serial.println(n);
#endif
    for (int8_t i = 0; i < n; ++i) {
      const char * ssid_scan = WiFi.SSID(i);
      int32_t rssi_scan = WiFi.RSSI(i);
      uint8_t sec_scan = WiFi.encryptionType(i);
      if (rssi_scan > maxRSSI) {
        maxRSSI = rssi_scan;
        strncpy_safe((char*)ssid_found, ssid_scan, MAX_SSID_LEN);
      }
#ifdef DEBUG
      Serial.print(ssid_scan);
      Serial.print(" ");
      Serial.print(encryptionTypeToStr(sec_scan));
      Serial.print(" ");
      Serial.println(rssi_scan);
#endif
      
      delay(0);
    }
  }
  return ssid_found;
}

void handleRoot() {
  webserver.send ( 200, "text/html", msg );
}

void handleNotFound() {
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += webserver.uri();
  message += "\nMethod: ";
  message += ( webserver.method() == HTTP_GET ) ? "GET" : "POST";
  message += "\nArguments: ";
  message += webserver.args();
  message += "\n";

  for ( uint8_t i = 0; i < webserver.args(); i++ ) {
    message += " " + webserver.argName ( i ) + ": " + webserver.arg ( i ) + "\n";
  }

  webserver.send ( 404, "text/plain", message );
}

void handleConfig() {
  ssid[0] = '\0';  // field 1
  security[0] = '\0'; // field 2
  password[0] = '\0'; // field 3
  //  portNo[0] = '\0'; // field 4
  staticIP[0] = '\0'; // field 5
  hostName[0] = '\0'; // field 6
  userName[0] = '\0'; // field 7
  userPw[0] = '\0'; // field 8
  uint16_t portNoInt = 80; // default

  byte mode = pfodFeatures::SERVER;
  uint16_t securityMode = pfodFeatures::WPA_WPA2;
  uint16_t portNo = 80;
  uint16_t ipSource = 0;

#ifdef DEBUG
  String message = "Config results\n\n";
  message += "URI: ";
  message += webserver.uri();
  message += "\nMethod: ";
  message += ( webserver.method() == HTTP_GET ) ? "GET" : "POST";
  message += "\nArguments: ";
  message += webserver.args();
  message += "\n";

  for ( uint8_t i = 0; i < webserver.args(); i++ ) {
    message += " " + webserver.argName ( i ) + ": " + webserver.arg ( i ) + "\n";
  }
  Serial.println(message);
#endif  

  uint8_t numOfArgs = webserver.args();
  const char *strPtr;
  uint8_t i = 0;
  for (; (i < numOfArgs); i++ ) {
    // check field numbers
    if (webserver.argName(i)[0] == '1') {
      strncpy_safe(ssid, (webserver.arg(i)).c_str(), pfodWifiConfig::MAX_SSID_LEN);
    } else if (webserver.argName(i)[0] == '3') {
      strncpy_safe(password, (webserver.arg(i)).c_str(), pfodWifiConfig::MAX_PASSWORD_LEN);
    } else if (webserver.argName(i)[0] == '4') {
      // convert portNo to uint16_6
      const char *portNoStr = (( webserver.arg(i)).c_str());
      long longPort = 0;
      pfodWifiConfig::parseLong((byte*)portNoStr, &longPort);
      portNo = (uint16_t)longPort;
    } else if (webserver.argName(i)[0] == '5') {
      strncpy_safe(staticIP, (webserver.arg(i)).c_str(), pfodWifiConfig::MAX_STATICIP_LEN);
      if (pfodWifiConfig::isEmpty(staticIP)) {
        // use dhcp
        staticIP[0] = '\0';
        ipSource = pfodFeatures::DHCP;
      }  else {
        ipSource = pfodFeatures::STATIC_IP;
      }
    }
  }


  pfodWifiConfig.storeValues( eepromAddress, mode, (byte *)ssid, (byte *)password, securityMode,
                              portNo, ipSource, (byte *)staticIP, (byte *)hostName, (byte *)userName, (byte *)userPw);
   delay(0);
  EEPROM.commit();
  delay(0);
  pfodWifiConfig.loadNetworkConfigFromEEPROM(eepromAddress, &mode,
      (char*)ssid, pfodWifiConfig::MAX_SSID_LEN + 1, (char*)password,  pfodWifiConfig::MAX_PASSWORD_LEN + 1,
      &securityMode, &portNo, &ipSource, (char*)staticIP,  pfodWifiConfig::MAX_STATICIP_LEN + 1);


  if (webserver.args() == 0) {
    webserver.send ( 200, "text/html", msg );
  } else {
    String rtnMsg = "<html>"
                    "<head>"
                    "<title>Server Setup</title>"
                    "<meta charset=\"utf-8\" />"
                    "<meta name=viewport content=\"width=device-width, initial-scale=1\">"
                    "</head>"
                    "<body>"
                    "<h2>Server Settings saved.<br>Power cycle to connect to ";
    rtnMsg += ssid;
    if (ipSource == pfodFeatures::DHCP) {
      rtnMsg += "<br> using DCHP to get its IP address";
    } else { // staticIP
      rtnMsg += "<br> using IP addess:";
      rtnMsg += staticIP;
    }
    rtnMsg += "<br> and start as server listening on port:";
    rtnMsg += portNo;
    rtnMsg += " .</h2>"
              "</body>"
              "</html>";

    webserver.send ( 200, "text/html", rtnMsg );
  }
}


const char WEP[] = "WEP";
const char TKIP[] = "TKIP";
const char CCMP[] = "CCMP";
const char NONE[] = "NONE";
const char AUTO[] = "WEP/WPA/WPA2";
const char UNKNOWN_ENCRY[] = "--UNKNOWN--";

const char* encryptionTypeToStr(uint8_t type) {
  if (type == ENC_TYPE_WEP) {
    return  WEP;
  } else if (type == ENC_TYPE_TKIP) {
    return  TKIP;
  } else if (type == ENC_TYPE_CCMP) {
    return  CCMP;
  } else if (type == ENC_TYPE_NONE) {
    return  NONE;
  } else if (type == ENC_TYPE_AUTO) {
    return  AUTO;
  } //else {
  return UNKNOWN_ENCRY;
}
