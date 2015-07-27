/**
 pfodWifiConfig for Arduino Compatibles
 http://www.forward.com.au/pfod/pfodWifiConfig/index.html

 (c)2015 Forward Computing and Control Pty. Ltd.
 This code may be freely used for both private and commerical use.
 Provide this copyright is maintained.
 */

#include "pfodWifiConfig.h"
#include <Arduino.h>
#include <EEPROM.h>

// if the next line is uncommented you need to call pfodWifiConfig::setDebugStream(&Serial);
//#define DEBUG

pfodWifiConfig::pfodWifiConfig() {
	debugOut = NULL;
}

/**
 * ipStrToNum
 * works for IPV4 only
 * return uint32_t for passing to ipAddress( );
 */
uint32_t pfodWifiConfig::ipStrToNum(const char* ipStr) {
  const int SIZE_OF_NUMS = 4;
  union {
	uint8_t bytes[SIZE_OF_NUMS];  // IPv4 address
	uint32_t dword;
  } _address;
  _address.dword = 0; // clear return

  int i = 0;
  uint8_t num = 0; // start with 0
  while ((*ipStr) != '\0') {
    // while not end of string
    if (*ipStr == '.') {
      // store num and move on to next position
      _address.bytes[i++] = num;
      num = 0;
      if (i>=SIZE_OF_NUMS) {
        break; // filled array
      }
    } else {  
      num = (num << 3) + (num << 1); // *10 = *8+*2
      num = num +  (*ipStr - '0');
    }  
    ipStr++;
  }  
  if (i<SIZE_OF_NUMS) {
    // store last num
    _address.bytes[i++] = num;
  }
  return _address.dword; 
}  

/**
 * parseLong
 * will parse between  -2,147,483,648 to 2,147,483,647
 * No error checking done.
 * will return 0 for empty string, i.e. first byte == null
 *
 * Inputs:
 *  idxPtr - byte* pointer to start of bytes to parse
 *  result - long* pointer to where to store result
 * return
 *   byte* updated pointer to bytes after skipping terminator
 *
 */
byte* pfodWifiConfig::parseLong(byte* idxPtr, long *result) {
  long rtn = 0;
  boolean neg = false;
  while ( *idxPtr != 0) {
    if (*idxPtr == '-') {
      neg = true;
    } else {
      rtn = (rtn << 3) + (rtn << 1); // *10 = *8+*2
      rtn = rtn +  (*idxPtr - '0');
    }
    ++idxPtr;
  }
  if (neg) {
    rtn = -rtn;
  }
  *result = rtn;
  return ++idxPtr; // skip null
}

void pfodWifiConfig::storeValues(int eepromAddress, byte mode, byte* ssidArg, byte* passwordArg, uint16_t selectedSecurity,
      uint16_t portNoLong, uint16_t ipSource, byte* staticIpArg, byte* hostArg, byte* userArg, byte* userPasswordArg) {
     char empty[] = ""; // this is not actually dereferenced
#ifdef DEBUG
  if (debugOut != NULL) {
  	debugOut->println("storeValue");
  	debugOut->println((const char*)ssidArg);
  	debugOut->println((const char*)passwordArg);
  	debugOut->println((const char*)staticIpArg);
  	debugOut->println(portNoLong);  	
  }
#endif
  // clear out any previous values. This sets the entire eeprom field len to 0
  storeNetworkConfigToEEPROM(eepromAddress, 0, (byte*) empty, (byte*) empty, 0, 0, 0, (byte*) empty,
             (byte*) empty, (byte*) empty, (byte*) empty);
  // save in eeprom
  storeNetworkConfigToEEPROM(eepromAddress, mode, ssidArg, passwordArg, (uint16_t) selectedSecurity,
            (uint16_t) portNoLong, (uint16_t) ipSource, staticIpArg, hostArg, userArg, userPasswordArg);
}

// will always put '\0\ at dest[maxLen]
// return the number of char copied excluding the terminating null
size_t pfodWifiConfig::strncpy_safe(char* dest, const char* src, size_t maxLen) {
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


void pfodWifiConfig::parseInput(pfodWifiConfigParser *parser, byte mode, uint16_t security, uint16_t ipSource, int eepromAddress) {
  char empty[] = ""; // this is not actually dereferenced

  byte cmd = parser->parse(); // pass it to the parser
  // parser returns non-zero when a pfod command is fully parsed
  // Note parser handles setSize cmd and replaces it with set + args
  // so here always have set as first arg if cmd valid
  if (cmd != 0) { // have parsed a complete msg { to }
    // now handle set command
    if ('.' == cmd) {
    	 outputFeatures(parser, mode, security, ipSource);
    } else if ('E' == cmd) {
      // error from parser
      char* msg = (char*) parser->getArg(0);
      if (strlen(msg) == 0) {
        sendMsg(parser->getPfodAppStream(), mode, true, "No working buffer set");
      } else {
        sendMsg(parser->getPfodAppStream(), mode, true, msg);
      }
    } else if (('s' == cmd) || ('S' == cmd)) { // pick up network settings
      // do some basic checks
#ifdef DEBUG
      if (debugOut != NULL) {
        debugOut->println();
        debugOut->print(F("Cmd:'"));
        byte* idxPtr = parser->getCmd();
        debugOut->print((char*) idxPtr);
        debugOut->println("'");
        debugOut->print(F("number of Args:"));
        debugOut->println(parser->getArgsCount());
        // parse result as chars
        for (int i=0;i<parser->getArgsCount();i++) {
        	debugOut->print('\'');
        	debugOut->print((char*)(parser->getArg(i)));
        	debugOut->println('\'');
        }
      }
#endif
      int argsCount = parser->getArgsCount();
      long portNoLong = 0;
      bool isValid = true;

      byte *cmd = parser->getCmd();
      int i = 0;
      while (cmd[i]) {
        cmd[i] = (char) tolower((int) cmd[i]);
        i++;
      }
      bool cmdOK = ((cmd[1] == 'e') && (cmd[2] == 't') && (cmd[3] == '\0'));
      if (!cmdOK) {
        isValid = false;
        sendMsg(parser->getPfodAppStream(), mode, true, "Unrecognized command.");
      }

      if ((mode < pfodFeatures::MIN_MODE) || (mode > pfodFeatures::MAX_MODE)) {
        // have other bit set here as well, args passed to configureWifiConfig in wrong order??
        isValid = false;
        sendMsg(parser->getPfodAppStream(), mode, true, "Invalid MODE");
      } else if (mode == pfodFeatures::SERVER) {
        if (ipSource == pfodFeatures::STATIC_IP) {
          // 5 args
          if (argsCount != 5) {
            isValid = false;
            sendMsg(parser->getPfodAppStream(), mode, true, "Invalid number of args");
          }
        } else if (ipSource == pfodFeatures::DHCP) {
          if (argsCount != 4) {
            isValid = false;
            sendMsg(parser->getPfodAppStream(), mode, true, "Invalid number of args");
          }
        } else {
          // valid args 4 or 5
          if ((argsCount != 4) && (argsCount != 5)) {
            isValid = false;
            sendMsg(parser->getPfodAppStream(), mode, true, "Invalid number of args");
          }
        }
      } else if (mode == pfodFeatures::CLIENT) {
        if (argsCount != 6) {
          isValid = false;
          sendMsg(parser->getPfodAppStream(), mode, true, "Invalid number of args.");
        }
      } else if (mode == pfodFeatures::CLIENTLOGIN) {
        if (argsCount != 8) {
          isValid = false;
          sendMsg(parser->getPfodAppStream(), mode, true, "Invalid number of args.");
        }
      } else {
        isValid = false;
        sendMsg(parser->getPfodAppStream(), mode, true, "UNKNOWN MODE");
      }

      if (isValid) {
        // get the args
        byte* ssidArg = parser->getArg(0);
        byte* securityArg = parser->getArg(1);
        byte* passwordArg = parser->getArg(2);
        byte* portNoArg = parser->getArg(3);
        byte* staticIpArg = (byte*) empty;
        byte* hostArg = (byte*) empty;
        byte* userArg = (byte*) empty;
        byte* userPasswordArg = (byte*) empty;
        if (argsCount > 4) {
          staticIpArg = parser->getArg(4);
        }
        if (argsCount > 5) {
          hostArg = parser->getArg(5);
        }
        if (argsCount > 6) {
          userArg = parser->getArg(6);
        }
        if (argsCount > 7) {
          userPasswordArg = parser->getArg(7);
        }
        size_t ssidLen = strlen((const char*) ssidArg);
        size_t passwordLen = strlen((const char*) passwordArg);
        /**
         // check for control chars
         if ((isValid) && (hasSpacesOrCtrlChars(ssidArg))) {
         isValid = false;
         sendMsg(parser->getPfodAppStream(), mode,true, "Spaces or Control characters found in SSID.");
         }
         if ((isValid) && (hasSpacesOrCtrlChars(passwordArg))) {
         isValid = false;
         sendMsg(parser->getPfodAppStream(),mode, true, "Spaces or Control characters found in password.");
         }
         **/
        if ((isValid) && (hasSpacesOrCtrlChars(portNoArg))) {
          isValid = false;
          sendMsg(parser->getPfodAppStream(), mode, true, "Spaces or Control characters found in portNo.");
        }
        if ((isValid) && (hasNonDigitChars(portNoArg))) {
          isValid = false;
          sendMsg(parser->getPfodAppStream(), mode, true, "portNo must be a number.");
        }

        if (isValid) {
          parser->parseLong(portNoArg, &portNoLong);
          if ((portNoLong <= 0) || (portNoLong > 65535)) {
            isValid = false;
            sendMsg(parser->getPfodAppStream(), mode, true, "Invalid portNo.");
          }
        }
        // check staticIp host user userpass
        if ((isValid) && (hasSpacesOrCtrlChars(staticIpArg))) {
          isValid = false;
          sendMsg(parser->getPfodAppStream(), mode, true, "Spaces or Control characters found in staticIP.");
        }
        if ((isValid) && (hasSpacesOrCtrlChars(hostArg))) {
          isValid = false;
          sendMsg(parser->getPfodAppStream(), mode, true, "Spaces or Control characters found in host to connect to.");
        }
        if ((isValid) && (hasSpacesOrCtrlChars(userArg))) {
          isValid = false;
          sendMsg(parser->getPfodAppStream(), mode, true, "Spaces or Control characters found in username.");
        }
        if ((isValid) && (hasSpacesOrCtrlChars(userPasswordArg))) {
          isValid = false;
          sendMsg(parser->getPfodAppStream(), mode, true, "Spaces or Control characters found in user password.");
        }

        if (isValid && ((ssidLen < 1) || (ssidLen > MAX_SSID_LEN))) {
          isValid = false;
          sendMsg(parser->getPfodAppStream(), mode, true, "Invalid NetworkSSID length, max 32 allowed.");
        }
        if (isValid && (passwordLen > MAX_PASSWORD_LEN)) {
          isValid = false;
          sendMsg(parser->getPfodAppStream(), mode, true, "Invalid password len, max 64 allowed.");
        }

        uint16_t selectedSecurity = pfodFeatures::WPA2;

        if (!pfodWifiConfig::convertSecurityToInt((char*) securityArg, &selectedSecurity)) {
          isValid = false;
          sendMsg(parser->getPfodAppStream(), mode, true, "Invalid security type.");
        }
        uint16_t ipSource = pfodFeatures::DHCP;

        if (!pfodWifiConfig::convertIPsourceToInt((char*) staticIpArg, &ipSource)) {
          isValid = false;
          sendMsg(parser->getPfodAppStream(), mode, true, "Invalid IP source type.");
        }

        if (isValid) {
          // clear out any previous values. This sets the entire eeprom field len to 0
          storeNetworkConfigToEEPROM(eepromAddress, 0, (byte*) empty, (byte*) empty, 0, 0, 0, (byte*) empty,
              (byte*) empty, (byte*) empty, (byte*) empty);
          // save in eeprom
          storeNetworkConfigToEEPROM(eepromAddress, mode, ssidArg, passwordArg, (uint16_t) selectedSecurity,
              (uint16_t) portNoLong, (uint16_t) ipSource, staticIpArg, hostArg, userArg, userPasswordArg);
          sendMsg(parser->getPfodAppStream(), mode, false,
              "Stored new network settings. Power cycle the device to connect");
#ifdef DEBUG
          if (debugOut != NULL) {
              char ssid[MAX_SSID_LEN + 1]; // allow for null also works for username
              char *username = ssid;
              char password[MAX_PASSWORD_LEN + 1];
              char host[MAX_HOSTNAME_LEN + 1];
              uint16_t portNo = 0;
              uint16_t ipSource = 0;
              byte mode = 0;
              uint16_t securityLoaded = 0;
              loadNetworkConfigFromEEPROM(eepromAddress, &mode, ssid, MAX_SSID_LEN + 1, password, MAX_PASSWORD_LEN + 1,
                  &securityLoaded, &portNo, &ipSource, host, MAX_HOSTNAME_LEN + 1);
              loadClientConfigFromEEPROM(eepromAddress, (char*) host, MAX_HOSTNAME_LEN + 1, username, MAX_SSID_LEN + 1,
                  password, MAX_PASSWORD_LEN + 1);
          }
#endif
        }
      }
    } else {
#ifdef DEBUG
      if (debugOut != NULL) {
        debugOut->println();
        debugOut->print(F("Cmd:'"));
        byte* idxPtr = parser->getCmd();
        debugOut->print((char*) idxPtr);
        debugOut->println("'");
      }
#endif
      sendMsg(parser->getPfodAppStream(), mode, true, "Unrecognized command.");
    }
  }
}

bool pfodWifiConfig::convertIPsourceToInt(char* ipSource, uint16_t*result) {
  // make all upper case and check against
  //DHCP
  if (strlen(ipSource) == 0) {
    *result = pfodFeatures::DHCP;
    return true;
  }
  int i = 0;
  while (ipSource[i]) {
    ipSource[i] = (char) toupper((int) ipSource[i]);
    i++;
  }
  int maxLen = 10;
  char str[maxLen];
  pfodWifiConfigParser::getProgStr(F("DHCP"), str, maxLen);
  if (strcmp(ipSource, str) == 0) {
    *result = pfodFeatures::DHCP;
    return true;
  } // else
  *result = pfodFeatures::STATIC_IP; // should do more checking here
  return true;
}

bool pfodWifiConfig::convertSecurityToInt(char* security, uint16_t*result) {
  // make all upper case and check against
  //OPEN,WEP,WPA,WPA2,WPA-WPA2
  int i = 0;
  while (security[i]) {
    security[i] = (char) toupper((int) security[i]);
    i++;
  }
  int maxLen = 10;
  char str[maxLen];
  pfodWifiConfigParser::getProgStr(F("OPEN"), str, maxLen);
  if (strcmp(security, str) == 0) {
    *result = pfodFeatures::OPEN;
    return true;
  } // else
  pfodWifiConfigParser::getProgStr(F("WEP"), str, maxLen);
  if (strcmp(security, str) == 0) {
    *result = pfodFeatures::WEP;
    return true;
  } // else
  pfodWifiConfigParser::getProgStr(F("WPA"), str, maxLen);
  if (strcmp(security, str) == 0) {
    *result = pfodFeatures::WPA;
    return true;
  } // else
  pfodWifiConfigParser::getProgStr(F("WPA2"), str, maxLen);
  if (strcmp(security, str) == 0) {
    *result = pfodFeatures::WPA2;
    return true;
  } // else
  pfodWifiConfigParser::getProgStr(F("WPA-WPA2"), str, maxLen);
  if (strcmp(security, str) == 0) {
    *result = pfodFeatures::WPA_WPA2;
    return true;
  } // else
  // else // assume WPA2
  return false;
}

void pfodWifiConfig::outputFeatures(pfodWifiConfigParser *parser, byte mode, uint16_t security, uint16_t ipSources) {
  char* str = (char*) (parser->getCmd());
  int maxLen = pfodWifiConfigParser::WORKING_BUFFER_SIZE;
  pfodWifiConfigParser::getProgStr(F("{!pfodWifiConfigV1 "), str, maxLen);
#ifdef DEBUG
  if (debugOut != NULL) {
    //debugOut->println(str);
  }
#endif
  parser->println(str);
  outputModeStr(parser, mode);
  outputSecurityStr(parser, security);
  outputIPsourceStr(parser, ipSources);
  parser->println('}');
}

void pfodWifiConfig::outputSecurityStr(pfodWifiConfigParser *parser, uint16_t security) {
  char* str = (char*) (parser->getCmd());
  int maxLen = pfodWifiConfigParser::WORKING_BUFFER_SIZE;
  pfodWifiConfigParser::getProgStr(F("Security: "), str, maxLen);
#ifdef DEBUG
  if (debugOut != NULL) {
    //debugOut->print(str);
  }
#endif
  parser->print(str);
  getSecurityStr(security, str, maxLen);
#ifdef DEBUG
  if (debugOut != NULL) {
    //debugOut->println(str);
  }
#endif
  parser->println(str);
}

void pfodWifiConfig::outputIPsourceStr(pfodWifiConfigParser *parser, uint16_t ipSource) {
  char* str = (char*) (parser->getCmd());
  int maxLen = pfodWifiConfigParser::WORKING_BUFFER_SIZE;

  pfodWifiConfigParser::getProgStr(F("IPsource: "), str, maxLen);
#ifdef DEBUG
  if (debugOut != NULL) {
    // debugOut->print(str);
  }
#endif
  parser->print(str);
  getIPsourceStr(ipSource, str, maxLen);
#ifdef DEBUG
  if (debugOut != NULL) {
    //debugOut->println(str);
  }
#endif
  parser->println(str);
}

void pfodWifiConfig::outputModeStr(pfodWifiConfigParser *parser, byte mode) {
  char* str = (char*) (parser->getCmd());
  int maxLen = pfodWifiConfigParser::WORKING_BUFFER_SIZE;

  pfodWifiConfigParser::getProgStr(F("Mode: "), str, maxLen);
#ifdef DEBUG
  if (debugOut != NULL) {
    //debugOut->print(str);
  }
#endif
  parser->print(str);
  getModeStr(mode, str, maxLen);
#ifdef DEBUG
  if (debugOut != NULL) {
    //debugOut->println(str);
  }
#endif
  parser->println(str);
}

char* pfodWifiConfig::getSecurityStr(uint16_t security, char*str, int maxLen) {
  *str = '\0'; // terminate to start with

  if (security & (~pfodFeatures::SECURITY_MASK)) {
    // have other bits set here as well, args passed to configureWifiConfig in wrong order??
    pfodWifiConfigParser::getProgStr(F("Invalid Security type"), str, maxLen);
  }
  // now handle OPEN|WEP|WPA|WPA2|WPA_WPA2
  // append each option
  int strLen = 0;
  if (security == 0) {
    pfodWifiConfigParser::getProgStr(F("INVALID Security type (zero)"), str + strLen, maxLen - strLen);
    return str;
  }
  if (security & pfodFeatures::OPEN) {
    pfodWifiConfigParser::getProgStr(F("OPEN"), str, maxLen);
    strLen = strlen(str);
    security = security & (~pfodFeatures::OPEN);  // mark this one off
  }
  if (security & pfodFeatures::WEP) {
    if (strLen != 0) {
      pfodWifiConfigParser::getProgStr(F(" | "), str + strLen, maxLen - strLen);
      strLen = strlen(str);
    }
    pfodWifiConfigParser::getProgStr(F("WEP"), str + strLen, maxLen - strLen);
    strLen = strlen(str);
    security = security & (~pfodFeatures::WEP); // mark this one off
  }
  if (security & pfodFeatures::WPA) {
    if (strLen != 0) {
      pfodWifiConfigParser::getProgStr(F(" | "), str + strLen, maxLen - strLen);
      strLen = strlen(str);
    }
    pfodWifiConfigParser::getProgStr(F("WPA"), str + strLen, maxLen - strLen);
    strLen = strlen(str);
    security = security & (~pfodFeatures::WPA); // mark this one off
  }
  if (security & pfodFeatures::WPA2) {
    if (strLen != 0) {
      pfodWifiConfigParser::getProgStr(F(" | "), str + strLen, maxLen - strLen);
      strLen = strlen(str);
    }
    pfodWifiConfigParser::getProgStr(F("WPA2"), str + strLen, maxLen - strLen);
    strLen = strlen(str);
    security = security & (~pfodFeatures::WPA2); // mark this one off
  }
  if (security & pfodFeatures::WPA_WPA2) {
    if (strLen != 0) {
      pfodWifiConfigParser::getProgStr(F(" | "), str + strLen, maxLen - strLen);
      strLen = strlen(str);
    }
    pfodWifiConfigParser::getProgStr(F("WPA-WPA2"), str + strLen, maxLen - strLen);
    strLen = strlen(str);
    security = security & (~pfodFeatures::WPA_WPA2); // mark this one off
  }
  if (security != 0) {
    // do we still have some bit set? if so this is a coding error
    if (strLen != 0) {
      pfodWifiConfigParser::getProgStr(F(" | "), str + strLen, maxLen - strLen);
      strLen = strlen(str);
    }
    pfodWifiConfigParser::getProgStr(F("UNKOWN Security type"), str + strLen, maxLen - strLen);
    strLen = strlen(str);
  }
  return str;
}

char* pfodWifiConfig::getIPsourceStr(uint16_t ipSource, char*str, int maxLen) {
  *str = '\0'; // terminate to start with

  if (ipSource & (~pfodFeatures::IP_SOURCE_MASK)) {
    // have other bits set here as well, args passed to configureWifiConfig in wrong order??
    pfodWifiConfigParser::getProgStr(F("Invalid IP source type"), str, maxLen);
  }
  // now handle DHCP|STATIC_IP
  // append each option
  int strLen = 0;
  if (ipSource == 0) {
    pfodWifiConfigParser::getProgStr(F("INVALID IP source type (zero)"), str + strLen, maxLen - strLen);
    return str;
  }
  if (ipSource & pfodFeatures::DHCP) {
    pfodWifiConfigParser::getProgStr(F("DHCP"), str, maxLen);
    strLen = strlen(str);
    ipSource = ipSource & (~pfodFeatures::DHCP);  // mark this one off
  }
  if (ipSource & pfodFeatures::STATIC_IP) {
    if (strLen != 0) {
      pfodWifiConfigParser::getProgStr(F(" | "), str + strLen, maxLen - strLen);
      strLen = strlen(str);
    }
    pfodWifiConfigParser::getProgStr(F("staticIP"), str + strLen, maxLen - strLen);
    strLen = strlen(str);
    ipSource = ipSource & (~pfodFeatures::STATIC_IP); // mark this one off
  }
  if (ipSource != 0) {
    // do we still have some bit set? if so this is a coding error
    if (strLen != 0) {
      pfodWifiConfigParser::getProgStr(F(" | "), str + strLen, maxLen - strLen);
      strLen = strlen(str);
    }
    pfodWifiConfigParser::getProgStr(F("UNKOWN IP source"), str + strLen, maxLen - strLen);
    strLen = strlen(str);
  }
  return str;
}

char* pfodWifiConfig::getModeStr(byte mode, char*str, int maxLen) {
  *str = '\0'; // terminate to start with
  if ((mode < pfodFeatures::MIN_MODE) || (mode > pfodFeatures::MAX_MODE)) {
    // have other bit set here as well, args passed to configureWifiConfig in wrong order??
    pfodWifiConfigParser::getProgStr(F("Invalid MODE"), str, maxLen);
  } else if (mode == pfodFeatures::SERVER) {
    pfodWifiConfigParser::getProgStr(F("Server"), str, maxLen);
  } else if (mode == pfodFeatures::CLIENT) {
    pfodWifiConfigParser::getProgStr(F("Client"), str, maxLen);
  } else if (mode == pfodFeatures::CLIENTLOGIN) {
    pfodWifiConfigParser::getProgStr(F("ClientLogin"), str, maxLen);
  } else {
    pfodWifiConfigParser::getProgStr(F("UNKNOWN MODE"), str, maxLen); // should not get here !!!
  }
  return str;
}

void pfodWifiConfig::setDebugStream(Print *out) {
  debugOut = out;
}

void pfodWifiConfig::loadClientConfigFromEEPROM(int eepromAddress, char*host, size_t maxHostlen, char* username,
    size_t maxUsernameLen, char*userPassword, size_t maxPwLen) {
  *host = '\0';
  *userPassword = '\0';
  *username = '\0';
  int len = readEEPROM(eepromAddress + hostOffset, (byte*) host, maxHostlen);
#ifdef DEBUG
  if (debugOut != NULL) {
    debugOut->print(F("host:"));
    debugOut->println((char*) host);
  }
#endif

  len = readEEPROM(eepromAddress + userNameOffset, (byte*) username, maxUsernameLen);
#ifdef DEBUG
  if (debugOut != NULL) {
    debugOut->print(F("username:"));
    debugOut->println((char*) username);
  }
#endif

  len = readEEPROM(eepromAddress + userPasswordOffset, (byte*) userPassword, maxPwLen);
#ifdef DEBUG
  if (debugOut != NULL) {
    debugOut->print(F("userPassword:"));
    debugOut->println((char*) userPassword);
  }
#endif
}

/**
 ** mode, security, ipSource values are defined in pfodFeatures class
 * use this call to get the network config for this device
 * If running as a Client
 * use loadClientConfigFromEEPROM to get hostToConnectTo and username and user password
 */
void pfodWifiConfig::loadNetworkConfigFromEEPROM(int eepromAddress, byte *mode, char*ssid, size_t maxSSIDlen,
    char*password, size_t maxPwLen, uint16_t *security, uint16_t*portNo, uint16_t *ipSource, char* staticIP,
    size_t maxStaticIPLen) {
  *ssid = '\0';
  *password = '\0';
  *staticIP = '\0';
  *portNo = 0;

#ifdef DEBUG
    int maxLen = 20;
    char tmpStr[maxLen];
#endif

  if (maxSSIDlen > MAX_SSID_LEN + 1) {
    maxSSIDlen = MAX_SSID_LEN + 1;
  }
  if (maxPwLen > MAX_PASSWORD_LEN + 1) {
    maxPwLen = MAX_PASSWORD_LEN + 1;
  }
  
  int len = readEEPROM(eepromAddress + ssidOffset, (byte*) ssid, maxSSIDlen);
#ifdef DEBUG
  if(debugOut != NULL) {
    debugOut->print(F("ssid:"));
    debugOut->println((char*) ssid);
  }
#endif

  len = readEEPROM(eepromAddress + passwordOffset, (byte*) password, maxPwLen);
#ifdef DEBUG
  if(debugOut != NULL) {
    debugOut->print(F("password:"));
    debugOut->println((char*) password);
  }
#endif

  len = readEEPROM(eepromAddress + staticIPoffset, (byte*) staticIP, maxStaticIPLen);
#ifdef DEBUG
  if(debugOut != NULL) {
    debugOut->print(F("staticIP:"));
    debugOut->println((char*) staticIP);
  }
#endif

  *mode = EEPROM.read(eepromAddress + modeOffset);
#ifdef DEBUG
  if(debugOut != NULL) {
    debugOut->print(F("mode:"));
    debugOut->println(getModeStr(*mode, tmpStr, maxLen));
  }
#endif

  union _uint16Bytes uintBytes;
  uintBytes.i = 0;
  int address = eepromAddress + portOffset;
  uintBytes.b[0] = EEPROM.read(address++);
  uintBytes.b[1] = EEPROM.read(address++);
  *portNo = uintBytes.i;
#ifdef DEBUG
  if(debugOut != NULL) {
    debugOut->print(F("portNo:"));
    debugOut->println(*portNo);
  }
#endif

  uintBytes.i = 0;
  address = eepromAddress + securityOffset;
  uintBytes.b[0] = EEPROM.read(address++);
  uintBytes.b[1] = EEPROM.read(address++);
  *security = uintBytes.i;
#ifdef DEBUG
  if(debugOut != NULL) {
    debugOut->print(F("security:"));
    debugOut->println(getSecurityStr(*security, tmpStr, maxLen));
  }
#endif
  uintBytes.i = 0;
  address = eepromAddress + ipSourceOffset;
  uintBytes.b[0] = EEPROM.read(address++);
  uintBytes.b[1] = EEPROM.read(address++);
  *ipSource = uintBytes.i;
#ifdef DEBUG
  if(debugOut != NULL) {
    debugOut->print(F("ipSource:"));
    debugOut->println(getIPsourceStr(*ipSource, tmpStr, maxLen));
  }
#endif

}

void pfodWifiConfig::sendMsg(Stream* parserPtr, byte mode, bool isError, const char* msg) {
  parserPtr->print(F("{!"));
  if (isError) {
    parserPtr->print(F("Error: "));
  } else {
    parserPtr->print(' ');
  }
  parserPtr->println(msg);
  if (isError) {
    parserPtr->println(F("Expected message has format:"));
    sendExpectedFormat(parserPtr, mode);
  }
  parserPtr->println('}');
}

void pfodWifiConfig::sendExpectedFormat(Stream* parserPtr, byte mode) {
  if ((mode < pfodFeatures::MIN_MODE) || (mode > pfodFeatures::MAX_MODE)) {
    // have other bit set here as well, args passed to configureWifiConfig in wrong order??
    parserPtr->print(F("Invalid MODE"));
  } else if (mode == pfodFeatures::SERVER) {
    parserPtr->print(F("{set <networkSSID> <security> <password> <portNo> <staticIP|DHCP>"));
  } else if (mode == pfodFeatures::CLIENT) {
    parserPtr->print(F("{set <networkSSID> <security> <password> <portNo> <staticIP|DHCP> <hostToConnectTo>"));

  } else if (mode == pfodFeatures::CLIENTLOGIN) {
    parserPtr->print(
        F(
            "{set <networkSSID> <security> <password> <portNo> <staticIP|DHCP> <hostToConnectTo> <username> <userPassword>"));
  } else {
    parserPtr->print(F("UNKNOWN MODE"));
  }
}

/**
 * Stores the network config in EEPROM
 * if any of the maxLen args are 0 then all the field are cleared with null bytes.
 */
void pfodWifiConfig::storeNetworkConfigToEEPROM(int eepromAddress, byte mode, byte* ssidArg, byte* passwordArg,
    uint16_t security, uint16_t portNoArg, uint16_t ipSource, byte* staticIP, byte* host, byte* username,
    byte* userPassword) {
  uint16_t portNo = portNoArg;
  size_t ssidLen = strlen((const char*) ssidArg);
  size_t passwordLen = strlen((const char*) passwordArg);
  size_t staticIPLen = strlen((const char*) staticIP);
  size_t hostLen = strlen((const char*) host);
  size_t userLen = strlen((const char*) username);
  size_t userPwLen = strlen((const char*) userPassword);

  if (ssidLen > MAX_SSID_LEN) {
    ssidLen = MAX_SSID_LEN;
  }
  if (passwordLen > MAX_PASSWORD_LEN) {
    passwordLen = MAX_PASSWORD_LEN;
  }
  if (staticIPLen > MAX_STATICIP_LEN) {
    staticIPLen = MAX_STATICIP_LEN;
  }
  if (hostLen > MAX_HOSTNAME_LEN) {
    hostLen = MAX_HOSTNAME_LEN;
  }
  if (userLen > MAX_USERNAME_LEN) {
    userLen = MAX_USERNAME_LEN;
  }
  if (userPwLen > MAX_USER_PASSWORD_LEN) {
    userPwLen = MAX_USER_PASSWORD_LEN;
  }
  if ((ssidLen == 0) || (mode == 0) || (passwordLen == 0) || (ipSource == 0) || (portNoArg == 0)) {
    portNoArg = 0;
    security = 0;
    mode = 0;
    ipSource = 0;
#ifdef DEBUG
    if(debugOut != NULL) {
      debugOut->println(F("Clearing EEPROM"));
    }
#endif
    clearEEPROM(eepromAddress + ssidOffset, MAX_SSID_LEN);
    EEPROM.write(eepromAddress + securityOffset, 0); // 1 == OPEN, 2 == WEP, 3 == WPA2 security
    clearEEPROM(eepromAddress + passwordOffset, MAX_PASSWORD_LEN);
    //
  } else {
    // write the new value
    writeEEPROM(eepromAddress + ssidOffset, ssidArg, ssidLen);
    EEPROM.write(eepromAddress + securityOffset, (uint8_t) 3); // 1 == OPEN, 2 == WEP, 3 == WPA2 security
    writeEEPROM(eepromAddress + passwordOffset, passwordArg, passwordLen);

    writeEEPROM(eepromAddress + staticIPoffset, staticIP, staticIPLen);
    writeEEPROM(eepromAddress + hostOffset, host, hostLen);
    writeEEPROM(eepromAddress + userNameOffset, username, userLen);
    writeEEPROM(eepromAddress + userPasswordOffset, userPassword, userPwLen);
  }
  //write mode
  EEPROM.write(eepromAddress + modeOffset, mode);

  // write ipSource
  union _uint16Bytes uintBytes;
  uintBytes.i = (uint16_t) ipSource;
  int address = eepromAddress + ipSourceOffset;
  EEPROM.write(address++, uintBytes.b[0]);
  EEPROM.write(address++, uintBytes.b[1]);
  // write secuity
  uintBytes.i = (uint16_t) security;
  address = eepromAddress + securityOffset;
  EEPROM.write(address++, uintBytes.b[0]);
  EEPROM.write(address++, uintBytes.b[1]);
  // write portNo
  uintBytes.i = (uint16_t) portNo;
  address = eepromAddress + portOffset;
  EEPROM.write(address++, uintBytes.b[0]);
  EEPROM.write(address++, uintBytes.b[1]);
}

/**
 * readEEPROM
 * maxLen is available storage for result (including null at end)
 */
byte pfodWifiConfig::readEEPROM(int eepromAddress, byte *key, byte maxLen) {
  byte len = EEPROM.read(eepromAddress++);
#ifdef DEBUG
  if(debugOut != NULL) {
    debugOut->print(F("read Len:"));
    debugOut->print(len);
    debugOut->print(F(" maxLen:"));
    debugOut->println(maxLen);
  }
#endif

  if (len >= maxLen) {
    return 0; // too long to fit in space proved
  }
  // else
  for (byte i = 0; i < len; i++) {
    *key = EEPROM.read(eepromAddress++);
#ifdef DEBUG
    if(debugOut != NULL) {
      debugOut->print(*((char*) key));
    }
#endif
    key++;
  }
#ifdef DEBUG
  if(debugOut != NULL) {
    debugOut->println();
  }
#endif
  *key = '\0'; // terminate
  return len;
}

/**
 *  check for byte !=32
 * return zero if one found
 */
byte pfodWifiConfig::isEmpty(const char* str) {
  for (; *str != 0; str++) {
    if ((*str != 32)) {
      return 0;
    }
  }
  return 1;
}

/**
 *  check for byte <=32 or > 127
 * return non-zero if one found
 */
byte pfodWifiConfig::hasSpacesOrCtrlChars(byte* str) {
  for (; *str != 0; str++) {
    if ((*str <= 32) || (*str > 127)) {
      return 1;
    }
  }
  return 0;
}

/**
 *  check for byte >'9' and <'0'
 * return non-zero if one found
 */
byte pfodWifiConfig::hasNonDigitChars(byte* str) {
  for (; *str != 0; str++) {
    if ((*str < '0') || (*str > '9')) {
      return 1;
    }
  }
  return 0;
}

/**
 * writeEEPROM  always writes len bytes.
 */
void pfodWifiConfig::writeEEPROM(int eepromAddress, byte *keyPtr, byte len) {
  EEPROM.write(eepromAddress++, len);
#ifdef DEBUG
  if(debugOut != NULL) {
    debugOut->print(F("wrote Len:"));
    debugOut->println(len);
  }
#endif

  for (int i = 0; i < len; i++) {
#ifdef DEBUG
    if(debugOut != NULL) {
      debugOut->print(*((char*) keyPtr));
    }
#endif
    EEPROM.write(eepromAddress++, *keyPtr++);
  }
#ifdef DEBUG
  if(debugOut != NULL) {
    debugOut->println();
  }
#endif
}

/**
 * clearEEPROM
 */
void pfodWifiConfig::clearEEPROM(int eepromAddress, byte len) {
  EEPROM.write(eepromAddress++, 0);
  for (int i = 0; i < len; i++) {
    EEPROM.write(eepromAddress++, 0);
  }
}
