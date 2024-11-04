#ifndef _literal_table_hpp_
#define _literal_table_hpp_
#include "string.h"
#include <string>
#include <iostream>
#include <vector>
using namespace std;

class LiteralTable{
public:
struct Literal{
  long int value;
  int size;
  int location;
  string symbol;
  Literal(long int v, int s, int l ):value(v),size(s),location(l),symbol(""){}
  Literal(long int v, int s, int l,string sym ):value(v),size(s),location(l),symbol(sym){}

};

vector<Literal> lits;

void addLiteral(long int v, int s, int l);
void addLiteral(long int v, int s, int l,string sym);
int getSize(){return lits.size();}
bool exists(int constant);
bool exists(string sym);

void print();
void make(int start)&;
long getLocation(long value);
long getLocation(string sym);

int getFullSize();

LiteralTable(){}

};

#endif