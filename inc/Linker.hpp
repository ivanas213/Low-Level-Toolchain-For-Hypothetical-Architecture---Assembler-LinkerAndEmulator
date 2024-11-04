#ifndef _linker_hpp_
#define _linker_hpp_
#include "SymbolTable.hpp"
#include "SectionTable.hpp"
#include <fstream>
#include "math.h"
#include <unordered_map>



class FlexLexer;

class Linker{
public:

struct SectionAddress{
  string section;
  string address;
  SectionAddress(string s,string a):section(s),address(a){}

};
private:
  static vector<string> input_files;
  static string output;
  Linker(vector<string> input,string output,vector<SectionAddress> sa,bool hex); 
  static vector<SymbolTable> symbolTables;
  static vector<SectionTable> sectionTables;
  static Linker* linker;
  static vector<SectionAddress> sectionAddress;
  static bool hex;
  static bool readingName;
  static bool readingNumber;
  static bool readingHexNumber;
  static bool readingSymbol;
  static const char* buffer;
  static FlexLexer* flexer;
  static SymbolTable* symbolTable;
  static SectionTable* sectionTable;
  static bool end;
  static bool error;
  static int nextIndex;
  static int startAddress;
  static int nextStart;
  static int currentSection;
public:
  static Linker init(vector<string> input, string output,vector<SectionAddress> sa,bool hex);
  static void setReadingName(){readingName=true;}
  static void putInBuffer(const char* b){buffer=b;}
  static void setReadingNumber(){readingNumber=true;}
  static void setReadingHexNumber(){readingHexNumber=true;}
  static void setReadingSymbol(){readingSymbol=true;}
  static void setEnd(){end=true;}
  static void link();
  static int pickNext();
  static int pickNextAndRow();
  static bool compareBuffer(const char* c);
  static int convertHex8ToNumber(string hex);
  static unsigned int convertHex8ToNumber(string hex,bool b);

  static char getHexChar(int number);
  static int getIntFromHexChar(char c);
  static void print();
  static void process();
  static void relocations();
  static string getHex(int number,int size);
  static string getHex(unsigned int number,int size);

  static bool noUndefined();

};
#endif