#ifndef pfodWifiConfig_ESP8266_h
#define pfodWifiConfig_ESP8266_h
/**
 pfodWifiConfig for Arduino Compatibles
 http://www.forward.com.au/pfod/pfodWifiConfig/index.html
 
 (c)2015 Forward Computing and Control Pty. Ltd.
 This code may be freely used for both private and commerical use.
 Provide this copyright is maintained.
 */

#include "pfodWifiConfig.h"

// ======================
class pfodWifiConfig_ESP8266: public pfodWifiConfig {
public:
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
  void configureAPconfig(int eepromAddress, const char* ssid, const char* password, int portNo, byte mode,
  uint16_t security, uint16_t ipSources);

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
  void configureWifiConfig(int eepromAddress, const char* ssid, const char* password, int portNo, byte mode,
  uint16_t security, uint16_t ipSources);
};


#endif
