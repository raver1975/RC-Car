#ifndef pfodWifiConfig_h
#define pfodWifiConfig_h
/**
 pfodWifiConfig for Arduino Compatibles
 http://www.forward.com.au/pfod/pfodWifiConfig/index.html
 
 (c)2015 Forward Computing and Control Pty. Ltd.
 This code may be freely used for both private and commerical use.
 Provide this copyright is maintained.
 */

#include "pfodWifiConfigParser.h"
#include <stdint.h>

// because these features are just bytes and bits the code has a lot of checking
// surrounding their use.  Errors are sent to the terminal on start up and
// whenever getFetures is called
class pfodFeatures {
public:
// splitting here means we can check if the arguments are in the correct order
  // these three are just a byte
  static const byte SERVER = 1;
  static const byte CLIENT = 2;
  static const byte CLIENTLOGIN = 3;
  static const byte MIN_MODE = SERVER;
  static const byte MAX_MODE = CLIENTLOGIN;  // for error checking
  // no overlap with mode settings
  // these are bit packed into an unsigend int but stored in EEPROM in a byte
  static const uint16_t OPEN = 4;
  static const uint16_t WEP = 8;
  static const uint16_t WPA = 16;
  static const uint16_t WPA2 = 32;
  static const uint16_t WPA_WPA2 = 64;
  static const uint16_t SECURITY_MASK = OPEN | WEP | WPA | WPA2 | WPA_WPA2; // for error checking

  // no overlap with IPsource settings
  // these are bit packed into an int
  // these are bit packed into an unsigend int but stored in EEPROM as text
  // either DHCP  or the actual static IP address
  static const uint16_t DHCP = 128;
  static const uint16_t STATIC_IP = 256;
  static const uint16_t IP_SOURCE_MASK = DHCP | STATIC_IP; // for error checking

};

class pfodWifiConfig {
protected:
  static const int userName_SSID_MaxLen = 32;
  static const int PASSWORD_MaxLen = 64;
  static const int IP_MaxLen = 40;
  static const int HOSTNAME_MaxLen = 64;
  static const int modeOffset = 0; // one byte for Mode
  static const int ipSourceOffset = modeOffset + 1; //  one byte for Mode,
  static const int securityOffset = ipSourceOffset + 2; //two byte for IP source,
  static const int ssidOffset = securityOffset + 2; //two bytes for security
  static const int passwordOffset = ssidOffset + userName_SSID_MaxLen + 1; // 32 ssid + len byte
  static const int portOffset = passwordOffset + PASSWORD_MaxLen + 1; // 64 bytes + len byte for password
  static const int staticIPoffset = portOffset + 2; // 2 bytes for port
  static const int userNameOffset = staticIPoffset + IP_MaxLen + 1; // 40 bytee for IP6 in text fromat or DHCP + len byte
  static const int userPasswordOffset = userNameOffset + userName_SSID_MaxLen + 1; // 32 bytes for username + len byte
  static const int hostOffset = userPasswordOffset + PASSWORD_MaxLen + 1; // 64 bytes for userPassword + len byte
  //static const int EndEEPROM = hostOffset + HOSTNAME_MaxLen + 1; // 64 bytes for hostid, + len byte

  byte readEEPROM(int eepromAddress, byte *key, byte maxLen);
  void writeEEPROM(int eepromAddress, byte *keyPtr, byte len);
  void clearEEPROM(int eepromAddress, byte len);
  byte hasSpacesOrCtrlChars(byte* str); // check for byte <32 or > 127, returns non zero if one found
  void sendMsg(Stream* parserPtr, byte mode, bool isError, const char* msg);
  void storeNetworkConfigToEEPROM(int eepromAddress, byte mode, byte* ssidArg, byte* passwordArg, uint16_t security,
      uint16_t portNoArg, uint16_t ipSource, byte* staticIP, byte* host, byte* username, byte* userPassword);

  union _uint16Bytes {
    uint16_t i;
    byte b[sizeof(uint16_t)]; // 2 bytes
  };
  void outputSecurityStr(pfodWifiConfigParser *parser, uint16_t security);
  void outputIPsourceStr(pfodWifiConfigParser *parser, uint16_t ipSource);
  void outputModeStr(pfodWifiConfigParser *parser, byte mode);
  void outputFeatures(pfodWifiConfigParser *parser, byte mode, uint16_t security, uint16_t ipSource);
  void parseInput(pfodWifiConfigParser *parser, byte mode, uint16_t security, uint16_t ipSource, int eepromAddress);
  void sendExpectedFormat(Stream* parserPtr, byte mode);
  bool convertSecurityToInt(char* security, uint16_t*result);
  bool convertIPsourceToInt(char* ipSource, uint16_t*result);
  byte hasNonDigitChars(byte* str);
  Print* debugOut;
public:
  pfodWifiConfig();
	void storeValues(int eepromAddress, uint8_t mode, byte* ssidArg, byte* passwordArg, uint16_t selectedSecurity,
      uint16_t portNoLong, uint16_t ipSource, byte* staticIpArg, byte* hostArg, byte* userArg, byte* userPasswordArg);
  static byte* parseLong(byte* idxPtr, long *result);
  static const int EndEEPROM = hostOffset + HOSTNAME_MaxLen + 1; // 64 bytes for hostid, + len byte
  
  /**
 *  check for byte !=32
 * return zero if one found
 */
  static byte isEmpty(const char* str);
  
   // will always put '\0' at dest[maxLen]
  static size_t strncpy_safe(char* dest, const char* src, size_t maxLen);

  static const int MAX_SSID_LEN = userName_SSID_MaxLen;
  static const int MAX_SECURITY_LEN = 12;  // "WPA/WPA2" say 12
  static const int MAX_PASSWORD_LEN = PASSWORD_MaxLen;
  static const int MAX_USERNAME_LEN = userName_SSID_MaxLen;
  static const int MAX_USER_PASSWORD_LEN = PASSWORD_MaxLen;
  static const int MAX_STATICIP_LEN = IP_MaxLen;
  static const int MAX_HOSTNAME_LEN = HOSTNAME_MaxLen;
  static const int MAX_PORTNO_STR_LEN = 5; // 65535
  static const int EEPROM_CLEAR_LENGTH = hostOffset; // don't clear the host but can change this to EndEEPROM if you want to clear host
  static const int EEPROM_USAGE = EndEEPROM; // 306 bytes. Note only 256 allowed for cmd {..} buffer
  void loadNetworkConfigFromEEPROM(int eepromAddress, byte *mode, char*ssid, size_t maxSSIDlen, char*password,
      size_t maxPwLen, uint16_t *security, uint16_t*portNo, uint16_t *ipSource, char* staticIP, size_t maxStaticIPLen);
  void loadClientConfigFromEEPROM(int eepromAddress, char*host, size_t maxHostlen, char* username,
      size_t maxUsernameLen, char*userPassword, size_t maxPwLen);

  void setDebugStream(Print *out);
  char* getModeStr(byte mode, char*str, int maxStrLen);
  char* getSecurityStr(uint16_t security, char*str, int maxLen);
  char* getIPsourceStr(uint16_t ipSource, char*str, int maxLen);
  static uint32_t ipStrToNum(const char* ipStr);
};

#endif
