#ifndef _relocation_table_hpp_
#define _relocation_table_hpp
#include <string>
#include <iostream>
#include <vector>
using namespace std;
class RelocationTable{
  public:
  /*enum Type{
    PC, NO_PC
  };*/
  struct Relocation{
    long int offset;
    string symbol;
    long int addend;
    Relocation(long int o,string s,int a):offset(o),symbol(s),addend(a){}
    void move(int m){offset+=m;}
  };
  
  
  vector<Relocation> relocations;
  
  RelocationTable(){}
  int getSize(){return relocations.size();}
  void addRelocation(long int offset,string symbol,int addend);
  void move(long address);
 // void addRelocation(Relocation r);
};

#endif