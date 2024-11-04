#ifndef _assembler_hpp_
#define _assembler_cpp_
#include <string>
#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>
#include "SymbolTable.hpp"
#include "SectionTable.hpp"
#include "LiteralTable.hpp"
#include <tr1/unordered_map>
#include <cstring>
#include "math.h"


using namespace std;
class FlexLexer;

class Assembler{
  public:
    enum Directive{
      global,extern_,section,word,skip,ascii,equ,end_
    };
    enum Command{
      halt, int_,iret, call,ret, jump,beq,bne,bgt,push,pop,xchg,add,sub,mul,div,not_,and_,or_,xor_,shl,shr,ld,st,csrrd,csrwr
    };
    enum Operand{
      literal_imm,symbol_imm,literal_mem,symbol_mem,reg_imm,reg_mem,reg_mem_lit,reg_mem_symb
    };
    enum Type{
      directive,command,operand,label
    };
    enum CommandType{
      zero_arg,one_arg,two_args,three_args
    };
    static Type currentAssembling;
  private:
    static SymbolTable* symbolTable;
    static SectionTable* sectionTable;
    static FlexLexer* flexer;
    static string currentsection;
    static int location_counter;
    static string input_file;
    static ifstream input_stream;
    static string output_file;
    static bool commentLine;
    static bool newLine;
    static bool readingSection;
    static bool readingCSR;
    static bool readingOperand;
    static int sectionCounter;
    
    static std::tr1::unordered_map<std::string, int> sections;
    //static std::tr1::unordered_map<char, short> ascii;

    static bool readingNumber;
    static bool end;
    static bool emptyLine;
    

    static string in;
    static const char* buffer;
    static bool error;
    static bool readingSymbols;
    static CommandType currentCommandType;
    static bool readingRegister;
    static bool readingLabel;
    static bool readingString;
    Assembler(string input, string output);
    static Assembler* assembler;
    static bool noSection;
    static RelocationTable relocationTable;
   // static LiteralTable* literalTable;
  public:
    static Assembler init(string input,string output);
    static void firstPass();
    static void mediumPass();
    static void secondPass();
    static void putInBuffer(const char* b);
    static void setComment(){commentLine=true;}
    static void resetComment(){commentLine=false;}
    static bool isReadingComment() {return commentLine;}
    static bool getComment(){return commentLine;}
    static void goToNewLine(){newLine=true;}
    static bool isNewLine(){return newLine;}
    static void setReadingSectionName(){readingSection=true;}
    static void addToSections(string sectionName);
    static void setReadingSymbols(){readingSymbols=true;}
    static void setReadingNumber(){readingNumber=true;}
    static void setCurrentCommandType(CommandType ct){currentCommandType=ct;}
    static CommandType getCommandType(){return currentCommandType;}
    static void setReadingRegister(){readingRegister=true;}
    static void setReadingCSR(){readingCSR=true;}
    static void setReadingOperand(){readingOperand=true;}
    static void setReadingLabel(){readingLabel=true;}
    static void setEnd(){end=true;}
    static void setCurrentAssembling(Type t){currentAssembling=t;}
    static void setEmptyLine(){emptyLine=true;}
    static void setReadingString(){readingString=true;}
    static int pickNext();
    static int pickNextAndRow();
    static int pickRow();
    static void setCurrentSection(string s){currentsection=s;}
    static vector<string> convertStringToHex(int number,int width);
    static string getHex(int number,int size);
    static char getHexChar(int number);
    static int convertToDecimal(string hex);
    static void processOperandFirstPass(string buffer);
    static void processSymbolFirstPass(string buffer);
    static void processLiteralFirstPass(string buffer);
    static int processOperandSecondPass(string buffer,string& reg,int& number,string& symbol_name);
    static int processSymbolSecondPass(string buffer);
    static int processLiteralSecondPass(string buffer);
    static int getNotChar(char c);
    static int getChar(char c);
    static void print();
    static char getHexCharInverted(int number);
    static string getHex3(int number, bool& error);
    static void addPoolCode();
    static bool checkForErrors();
};

#endif