#ifndef pfodWifiConfigParser_h
#define pfodWifiConfigParser_h
/**
 pfodWifiConfig for Arduino Compatibles
 http://www.forward.com.au/pfod/pfodWifiConfig/index.html
 
 (c)2015 Forward Computing and Control Pty. Ltd.
 This code may be freely used for both private and commerical use.
 Provide this copyright is maintained.
 */

#include "Arduino.h"
#include "Stream.h"
// ======================
class pfodWifiConfigParser: public Print {
  public:
    pfodWifiConfigParser();
    void setWorkingBuffer(byte *buffer, int bufferLen); // set the working buffer
    void connect(Stream* ioPtr);
    byte parse();
    byte* getCmd();
    byte* getArg(byte idx);
    byte getArgsCount();
    byte* parseLong(byte* idxPtr, long *result);
    static const int WORKING_BUFFER_SIZE = 256; // default size of the working buffer from getCmd()
    void setDebugStream(Print* debugOut);
    size_t write(uint8_t c);
    void flush();
    Stream* getPfodAppStream(); // get the command response stream we are writing to
    static int getProgStr(const __FlashStringHelper *ifsh, char*str, int maxLen);
    size_t write(const uint8_t *buffer, size_t size);
  private:
    void init();
    byte parse(byte in);
    byte* getFirstArg();
    void setCmd(byte cmd);
    char getParserStateChar(); // only used in DEBUG

    static const byte colon = (byte)':';
    static const byte pfodWaitingForStart = 0xff;
    static const byte pfodMsgStarted = '{';
    static const byte pfodInCmd = 0;
    static const byte pfodMsgEnd = '}';
    static const byte pfodWaitingForArg = ' ';
    static const byte pfodInArg = 0xfe;
    static const byte pfodInSize = 0xfd;
    static const byte pfodStoreField = 0xfc;
    void setUpErrorReturn();
    void addToSize(int *fieldSize, byte in);
    bool isDigit(byte b);
    bool isSizeCmd; // = false;
    int fieldSize;// = 0;
    Print* debugStream;
    Stream* io;
    const static int pfodMaxMsgLen = WORKING_BUFFER_SIZE-1; 
    byte args[pfodMaxMsgLen]; 
    byte argsCount;  // no of arguments found in msg
    int argsIdx;
    byte parserState;
};

#endif


