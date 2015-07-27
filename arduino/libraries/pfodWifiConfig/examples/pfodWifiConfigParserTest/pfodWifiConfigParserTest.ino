/**
 * pfodWifiConfig for Arduino Compatibles
 * http://www.forward.com.au/pfod/pfodWifiConfig/index.html
 * 
 * (c)2015 Forward Computing and Control Pty. Ltd.
 * This code may be freely used for both private and commerical use.
 * Provide this copyright is maintained.
 */

/**
 * Test sketch for pfodWifiConfigParser
 * Compile for UNO and then enter either of the two set commands
 * {set <ssid> <security> <password> <portNo>}
 * OR if ssid or password contains spaces or the '}' character
 * {<numberSize>:<ssid> <security> <numberSize>:<password> <portNo>}
 * 
 * E.g.
 * {set mySSID WPA2-PSK myPassword 80}
 * OR
 * {7:my SSID WPA2-PKS 10:prz3}!~34a 80}
 * 
 */
#include "pfodWifiConfig.h"
#include <EEPROM.h>
pfodWifiConfigParser parser;
// if not using pfodParser or pfodSecurity
// allocate a working buffer for pfodWifiConfig use
byte workingBuffer[pfodWifiConfigParser::WORKING_BUFFER_SIZE];

void setup() {
  // Open serial communications and wait for port to open:
  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for Leonardo only
  }
  delay(1000);
  // allow a little time to connect the serialMonitor before running the rest of the setup.
  for (int i = 10; i > 0; i--) {
    delay(500);
    Serial.print(F(" "));
    Serial.print(i);
  }
  Serial.println();
  Serial.println(F("pfodWifiConfigParser ready, enter messages via serial monitor (9600baud)"));
  Serial.println(F("of the form"));
  Serial.println(F("   {set <ssid> <security> <password> <portNo>}"));
  Serial.println(F("       with a space between the fields."));
  Serial.println(F("OR if any field contains spaces or the '}' character, then use this from"));
  Serial.println(F("   {<numberSize>:<ssid> <numberSize>:<security> <numberSize>:<password> <numberSize>:<portNo>}"));
  Serial.println(F(" The <numberSize> is the length of the following field.  If any field has length, then all fields must have a length."));
  Serial.println(F(" There must be a space after the end of each field, before the next field"));  
  Serial.println(F(" Note: In the <numberSize> format there is no 'set' after the opening {"));
  Serial.println(); 
  Serial.println(F("E.g."));
  Serial.println(F(" {set mySSID WPA2-PSK myPassword 80}"));
  Serial.println(F("OR"));
  Serial.println(F(" {7:my SSID 8:WPA2-PKS 10:prz3}!~34a 2:80}"));
  Serial.println(); 

  //parser.setDebugStream(&Serial); //add this line if using DEBUG in the library code files
  parser.connect(&Serial);  // connect the parser to read and write to Serial (USB)
}

void loop() { // run over and over
  byte cmd = 0;
  cmd = parser.parse();
  if (cmd != 0) {
    parser.println();
    parser.print(F("Cmd:'"));
    byte* idxPtr = parser.getCmd();
    parser.print((char*)idxPtr);
    parser.println("'");
    parser.print(F("number of Args:"));
    parser.println(parser.getArgsCount());
    // parse result as chars
    for (int i=0;i<parser.getArgsCount();i++) {
      parser.print('\'');
      parser.print((char*)parser.getArg(i));
      parser.println('\'');
    }
    // so clear it
    cmd = 0; // have processed this cmd now
    // so clear it and wait for next one
  }
  // else keep looping
}



