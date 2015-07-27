/**
 pfodWifiConfig for Arduino Compatibles
 http://www.forward.com.au/pfod/pfodWifiConfig/index.html

 (c)2015 Forward Computing and Control Pty. Ltd.
 This code may be freely used for both private and commerical use.
 Provide this copyright is maintained.
 */

#include "pfodWifiConfigParser.h"

//#define DEBUG
// MUST CALL setDebugStream() if line above is un-commented!!

//=========================================================================

void pfodWifiConfigParser::addToSize(int *fieldSize, byte in) {
  int num = *fieldSize;
  num = (num << 3) + (num << 1); // *10 = *8+*2
  num = num + (in - ((byte) '0'));
  *fieldSize = num;
#ifdef DEBUG
  if (debugStream != NULL) {
    debugStream->print("new field size:");
    debugStream->println(*fieldSize);
  }
#endif
}

void pfodWifiConfigParser::setUpErrorReturn() {
  init(); // no args
  setCmd((byte) 'E');
  argsCount = 1;
}

/**
 * parse
 * Inputs:
 *
 * States:
 * before {   parserState == pfodWaitingForStart
 * when   { seen parserState == pfodInCmd
 */
byte pfodWifiConfigParser::parse(byte in) {
#ifdef DEBUG
  if (debugStream != NULL) {
    debugStream->print("pfodMaxMsgLen:");
    debugStream->print(pfodMaxMsgLen);
    debugStream->print("  isSizeCmd:");
    debugStream->print(isSizeCmd);
    debugStream->print("  parserState:");
    debugStream->print(getParserStateChar());
    debugStream->print(" in:");
    debugStream->println((char) in);
  }
#endif

  // parser state
  // pfodWaitingForStart before {
  // pfodMsgEnd have seen }  terminate any arg or cmd and add second null to terminate getNextArg()
  // pfodMsgStarted have seen { skip chars until non space
  // if inCmd or inArg and see ' ' then terminate and goto waitingForArg
  // if waitingForArg skip ' ' if see non ' ' goto inArg and increment arg count
  // if pfodMsgStarted and see non ' ' check for digit and is so set isSizeCmd
  // if error set cmd to E and fill arg with error msg

  if (in == 0xff) {
    // note 0xFF is not a valid utf-8 char
    // but is returned by Stream.read() if not data available
    // should not normally happen
    return 0;
  }
  if (parserState == pfodWaitingForStart) {
    if (in == pfodMsgStarted) { // found {
      init(); // clean out last cmd
      parserState = pfodMsgStarted;
    }
    // else ignore this char as waiting for start {
    // always reset counter if waiting for {
    return 0;
  }

  // else have seen {
  if ((argsIdx >= (pfodMaxMsgLen - 2)) && (in != pfodMsgEnd)) {
    // note { } not stored so -2
    setUpErrorReturn();
    // fill in error msg
    getProgStr(F("Ignoring message as it is longer than 254 bytes"), (char*) (args + 2), pfodMaxMsgLen - 2);
    return args[0];
  }

  // else process this msg char
  // look for special chars ' ' and  }
  if (((in == pfodMsgEnd) && (parserState != pfodStoreField)) || // test for closing } here
  (((parserState == pfodInArg) || (parserState == pfodInCmd)) && (in == (byte) ' '))) {
    // terminate arg,cmd or message
#ifdef DEBUG
    if (debugStream != NULL) {
      debugStream->print("put null at ");
      debugStream->println(argsIdx);
    }
#endif
    args[argsIdx++] = 0; // termnate
    if ((parserState == pfodInArg)) {
      // increase count if was parsing arg
      argsCount++;
    }
    if (in == pfodMsgEnd) {
      parserState = pfodWaitingForStart; // reset state
#ifdef DEBUG
          if (debugStream != NULL) {
            debugStream->print("put null at ");
            debugStream->println(argsIdx);
          }
#endif
      args[argsIdx++] = 0; // termnate
#ifdef DEBUG
          if (debugStream != NULL) {
            debugStream->print("argsIdx:");
            debugStream->println(argsIdx);
          }
#endif
      // return command byte found
      // this will return 0 when parsing {} msg
      return args[0];
    } else { // was in cmd or arg set state to skip ' '
      parserState = pfodWaitingForArg;
    }
    return 0;
  }

  if (parserState == pfodStoreField) {
#ifdef DEBUG
    if (debugStream != NULL) {
      debugStream->print("field size:");
      debugStream->println(fieldSize);
    }
#endif
    if (fieldSize > 0) {
      fieldSize--;
      // continue to store at bottom
    } else {
      // == 0 this char should be ' ' or '}'
      if (in == pfodMsgEnd) {
        // found '}' after end of field
        parserState = pfodWaitingForStart; // reset state
#ifdef DEBUG
            if (debugStream != NULL) {
              debugStream->print("put null at ");
              debugStream->println(argsIdx);
            }
#endif
        args[argsIdx++] = 0; // termnate
#ifdef DEBUG
            if (debugStream != NULL) {
              debugStream->print("argsIdx:");
              debugStream->println(argsIdx);
            }
#endif
        argsCount++; // finished last arg
        // return command byte found
        return args[0];
      }
      if (in != ' ') {
        setUpErrorReturn();
        // fill in error msg
        getProgStr(F("Field with size, was not followed by a space."), (char*) (args + 2), pfodMaxMsgLen - 2);
        return args[0];
      }
      // else found space after ssid
#ifdef DEBUG
      if (debugStream != NULL) {
        debugStream->print("end of ssid put null at ");
        debugStream->println(argsIdx);
      }
#endif
      argsCount++;
      args[argsIdx++] = 0; // termnate
#ifdef DEBUG
          if (debugStream != NULL) {
            debugStream->print("argsIdx:");
            debugStream->println(argsIdx);
          }
#endif
      parserState = pfodWaitingForArg;
      return 0; //
    }
  }

  if (parserState == pfodInSize) {
    if (in == colon) {
      if (fieldSize == 0) {
#ifdef DEBUG
        if (debugStream != NULL) {
          debugStream->println(" field size 0");
        }
#endif
      }
      // start counting down ssid size
      parserState = pfodStoreField;
      return 0;

    } else {
      if (!isDigit(in)) {
        setUpErrorReturn();
        // fill in error msg
        getProgStr(F("Invalid Field size, non-digit found."), (char*) (args + 2), pfodMaxMsgLen - 2);
        return args[0];
      } // else
      // add to size
      addToSize(&fieldSize, in);
      if (fieldSize > (pfodMaxMsgLen - 2 - argsIdx)) {
#ifdef DEBUG
        if (debugStream != NULL) {
          debugStream->print("field size:");
          debugStream->print(fieldSize);
          debugStream->print(" larger then:");
          debugStream->println((pfodMaxMsgLen - 2 - argsIdx));
        }
#endif
        // size to large for message buffer
        setUpErrorReturn();
        // fill in error msg
        getProgStr(F("Field size larger then remaining buffer size"), (char*) (args + 2), pfodMaxMsgLen - 2);
        return args[0];
      }
    }
    return 0; // don't store digits in buffer
  }

  // first char after opening {
  if (parserState == pfodMsgStarted) {
    // skip blanks
    if (in == ' ') {  // tested for msgEnd above
      return 0;
    } else {
      if (isDigit(in)) {
#ifdef DEBUG
        if (debugStream != NULL) {
          debugStream->print("Found leading digit so SetSize format ");
          debugStream->println((char) in);
        }
#endif
        isSizeCmd = true;
        fieldSize = 0;
        addToSize(&fieldSize, in);
        if (fieldSize > (pfodMaxMsgLen - 2 - argsIdx)) {
#ifdef DEBUG
          if (debugStream != NULL) {
            debugStream->print("field size:");
            debugStream->print(fieldSize);
            debugStream->print(" larger then:");
            debugStream->println((pfodMaxMsgLen - 2 - argsIdx));
          }
#endif
          // size to large for message buffer
          setUpErrorReturn();
          // fill in error msg
          getProgStr(F("Field size larger then remaining buffer size"), (char*) (args + 2), pfodMaxMsgLen - 2);
          return args[0];
        }
        parserState = pfodInSize;
        args[argsIdx++] = 's';
        args[argsIdx++] = 'e';
        args[argsIdx++] = 't';
        args[argsIdx++] = '\0'; // initialize cmd return
        return 0;
      } else {  // not digit is set commmand
        parserState = pfodInCmd; // set cmd save byte below
      }
    }
  }

  if (parserState == pfodWaitingForArg) {
    if (in == ' ') {
      // skip blanks
      return 0;
    } else {
      if (isSizeCmd) {
        // expect field length to start each field
        if (isDigit(in)) {
#ifdef DEBUG
          if (debugStream != NULL) {
            debugStream->print("Found leading field digit ");
            debugStream->println((char) in);
          }
#endif
          fieldSize = 0;
          addToSize(&fieldSize, in);
          if (fieldSize > (pfodMaxMsgLen - 2 - argsIdx)) {
#ifdef DEBUG
            if (debugStream != NULL) {
              debugStream->print("field size:");
              debugStream->print(fieldSize);
              debugStream->print(" larger then:");
              debugStream->println((pfodMaxMsgLen - 2 - argsIdx));
            }
#endif
            // size to large for message buffer
            setUpErrorReturn();
            // fill in error msg
            getProgStr(F("Field size larger then remaining buffer size"), (char*) (args + 2), pfodMaxMsgLen - 2);
            return args[0];
          }
          parserState = pfodInSize;
          return 0;
        } else {  // not digit but isSizeCmd
          setUpErrorReturn();
          // fill in error msg
          getProgStr(F("All fields must have a field size if the first field does."), (char*) (args + 2),
              pfodMaxMsgLen - 2);
          return args[0];
        }
      } else {
        parserState = pfodInArg; // save byte below
      }
    }
  }
  // else normal byte
#ifdef DEBUG
  if (debugStream != NULL) {
    debugStream->print("put ");
    debugStream->print((char) in);
    debugStream->print(" at ");
    debugStream->println(argsIdx);
  }
#endif
  args[argsIdx++] = in;
  return 0;
}

bool pfodWifiConfigParser::isDigit(byte b) {
  return (('0' <= b) && (b <= '9'));
}

// maxLen includes null at end
int pfodWifiConfigParser::getProgStr(const __FlashStringHelper * ifsh, char * str, int maxLen) {
  const char *p = (const char *)ifsh;
  if (maxLen < 1) {
    return 0;
  }
  int i = 0;
  while (i < maxLen - 1) {
    unsigned char c = pgm_read_byte(p++);
    str[i++] = c;
    if (c == 0) {
      break;
    }
  }
  str[i] = '\0'; // terminate
  return i;
}

pfodWifiConfigParser::pfodWifiConfigParser() {
  io = NULL;
  debugStream = NULL;
  init();
}

/**
 * Note: this must NOT null the io stream
 */
void pfodWifiConfigParser::init() {
  fieldSize = 0;
  isSizeCmd = false;
  argsCount = 0;
  argsIdx = 0;
  args[0] = 0; // no cmd yet
  args[1] = 0; //
  parserState = pfodWaitingForStart; // not started yet pfodInCmd when have seen {
}

void pfodWifiConfigParser::connect(Stream * ioPtr) {
  init();
  io = ioPtr;
}

Stream* pfodWifiConfigParser::getPfodAppStream() {
  return io;
}

size_t pfodWifiConfigParser::write(uint8_t c) {
  if (!io) {
    return 1; // cannot write if io null but just pretend to
  }
  return io->write(c);
}

void pfodWifiConfigParser::flush() {
  if (!io) {
    return; // cannot write if io null but just pretend to
  }
  io->flush();
}


size_t pfodWifiConfigParser::write(const uint8_t *buffer, size_t size) {
  if (!io) {
    return 1;
  }
  return io->write(buffer, size);
}


void pfodWifiConfigParser::setCmd(byte cmd) {
  init();
  args[0] = cmd;
  args[1] = 0;
}

/**
 * Return pointer to start of args[]
 */
byte* pfodWifiConfigParser::getCmd() {
  return args;
}

/**
 * Return pointer to first arg (or pointer to null if no args)
 *
 * Start at args[0] and scan for first null
 * if argsCount > 0 increment to point to  start of first arg
 * else if argsCount == 0 leave pointing to null
 */
byte* pfodWifiConfigParser::getFirstArg() {
  byte* idxPtr = args;
  while (*idxPtr != 0) {
    ++idxPtr;
  }
  if (argsCount > 0) {
    ++idxPtr;
  }
  return idxPtr;
}

/**
 * Return pointer to this arg or pointer to null idx >= getArgsCount()
 * idx is zero based and goes from 0 to getArgsCount()-1
 */
byte* pfodWifiConfigParser::getArg(byte idx) {
  byte* idxPtr = getFirstArg();
  byte count = 0;
  while (count < idx) {
    while (*idxPtr != 0) {
      ++idxPtr;
    }
    count++;
    if (count < getArgsCount()) {
      ++idxPtr; // skip null
    } else {
      return idxPtr;
      // else this was the last arg // just return null pointer
    }
  }
  return idxPtr;
}

/**
 * Return number of args in last parsed msg
 */
byte pfodWifiConfigParser::getArgsCount() {
  return argsCount;
}

// only used for debug
char pfodWifiConfigParser::getParserStateChar() {
#ifdef DEBUG
  if (debugStream != NULL) {
    if (parserState == pfodWaitingForStart) {
      return 'B';
    }
    if (parserState == pfodMsgStarted) {
      return '{';
    }
    if (parserState == pfodInCmd) {
      return 'C';
    }
    if (parserState == pfodStoreField) {
      return 'N';
    }
    if (parserState == pfodInSize) {
      return 'S';
    }
    if (parserState == pfodWaitingForArg) {
      return 'W';
    }
    if (parserState == pfodInArg) {
      return 'I';
    }

    if (parserState == pfodMsgEnd) {
      return '}';
    } else {
      return 'U';
    } //else {
  }
#endif
  return ' '; // no debug
}

byte pfodWifiConfigParser::parse() {
  byte rtn = 0;
  if (!io) {
    return rtn;
  }
  while (io->available()) {
    int in = io->read();
    rtn = parse((byte) in);
    if (rtn != 0) {
      // found cmd
      return rtn;
    }
  }
  return rtn;
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
byte* pfodWifiConfigParser::parseLong(byte * idxPtr, long * result) {
  long rtn = 0;
  boolean neg = false;
  while (*idxPtr != 0) {
    if (*idxPtr == '-') {
      neg = true;
    } else {
      rtn = (rtn << 3) + (rtn << 1); // *10 = *8+*2
      rtn = rtn + (*idxPtr - '0');
    }
    ++idxPtr;
  }
  if (neg) {
    rtn = -rtn;
  }
  *result = rtn;
  return ++idxPtr; // skip null
}

void pfodWifiConfigParser::setDebugStream(Print * debugOut) {
  debugStream = debugOut;
}

