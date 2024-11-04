#ifndef _symbol_table_hpp_
#define _symbol_table_hpp_
#include <iostream>
#include <string>
#include <vector>
#include <iostream>
#include "string.h"
#include <iomanip>

using namespace std;


class SymbolTable{
  public:
    enum Bind{
    LOCAL, GLOBAL, NOBIND
    };
    enum Type{
    NOTYPE, SECTION, FUNC, OBJECT
    };
  
  struct Symbol{
    int index;
    int value;
    int size;
    Type type;
    Bind bind;
    string ndx;
    string name;
    bool isExtern;
      //int,string,int,type,bool,section,string

    Symbol(int i,int v, int s, Type t, Bind b, string n, string nm):index(i), value(v), size(s),type(t),bind(b),ndx(n),name(nm),isExtern(false){}
    void setAddress(int a){value=a;}
    void move(int a){value=value+a;}
  };
  vector<Symbol> symbols;
    SymbolTable(){}
    void insertNewSymbol(string naziv, int vrednost, int velicina, Type tip,Bind povezivanje, string sekcija);
    bool isDeclared(string symbol);
    bool isDefined(string symbol);
    int getSize(){return symbols.size();}
    void setGlobal(string name);
    void setLocal(string name);
    bool haveBind(string name);
    void define(string symbol,int location,string section);
    void print(ostream& os);
    int getValue(string symbol);
    Bind getBind(string symbol);
    string getSection(string symbol);
    Type getType(string symbol);
    void setExtern(string name);
    bool isExtern(string name);
    bool checkForErrors();
    void setSection(string name,string section);
    Symbol& getSymbol(int i);
    void processForLinker();
    void moveSymbol(int sym, int addr);
};

#endif