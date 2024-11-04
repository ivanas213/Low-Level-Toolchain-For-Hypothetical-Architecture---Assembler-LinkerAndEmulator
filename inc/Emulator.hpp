#ifndef _emulator_hpp_
#define _emulator_hpp_
#include <iostream>
#include <fstream>
#include <tr1/unordered_map>
#include "string.h"
#include <string>
#include "math.h"
#include <iomanip>


using namespace std;

class FlexLexer;

class Emulator{
  Emulator(string input);
  static Emulator* emulator;
  static string input;
  static const char* buffer;
  static bool readingAddress;
  static bool readingHex;
  static bool end;
  static ifstream input_stream;
  static FlexLexer* flexer;
  static std::tr1::unordered_map<uint32_t, uint8_t> memory;
  static int32_t registers[16];
 
  static int32_t status;
  static int32_t cause;
  static int32_t handler;
  static bool error;
  static bool interrupt;
  static bool mem;
  static int a,b,c,d;
  static uint operation;


public:
  static Emulator init(string input);
  static void putInBuffer(const char* b){buffer=b;}
  static void setReadingAddress(){readingAddress=true;}
  static void setReadingHex(){readingHex=true;}
  static void setEnd(){end=true;}
  static void job();
  static void load();
  static int hexToInt(string s);
  static unsigned int hexToUInt(string s);

  static string getHex(int number,int size);
  static char getHexChar(int number);
  static char getHexCharInverted(int number);
  static void parseNextInstruction();

  static void processHalt();
  static void processSoftwareInterrupt();
  static void processStartSubroutine();
  static void processStartSubroutineMem();
  static void processJump();
  static void processBEQ();
  static void processBNE();
  static void processBGT();
  static void processJumpMem();
  static void processBEQMem();
  static void processBNEMem();
  static void processBGTMem();
  static void processChange();
  static void processAdd();
  static void processSub();
  static void processMul();
  static void processDiv();
  static void processNot();
  static void processAnd();
  static void processOr();
  static void processXor();
  static void processShiftLeft();
  static void processShiftRight();
  static void processStore();
  static void processStoreStore();
  static void processStoreAdd();
  static void processLdGrpCsr();
  static void processLdGprGpr();
  static void processLdGprGprMem();
  static void processLdGprMemAdd();
  static void processCsrGrp();
  static void processCsrCsrD();
  static void processCsrMem();
  static void processCsrMemAdd();
  static void push(int32_t val);
  static void push(uint32_t val);
  static void print();
  static int getIntFromHexChar(char c);
  static void convertD();
  static int8_t convertStringByteToInt8(string b);
  static uint32_t getFromMemory(int32_t address);
  static int32_t getFromMemory(int32_t address,bool b);

  static void putToMemory(uint32_t address, uint32_t data);
};

#endif