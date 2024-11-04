#include "../inc/LiteralTable.hpp"

bool LiteralTable::exists(int constant){
  for(int i=0;i<lits.size();i++){
    if(lits[i].value==constant && !(strcmp(lits[i].symbol.c_str(),""))) return true;
  }
  return false;
}
bool LiteralTable::exists(string sym){
  for(int i=0;i<lits.size();i++){
    if(lits[i].symbol==sym) return true;
  }
  return false;
}

void LiteralTable::addLiteral(long int v, int s, int l){
  lits.push_back(Literal(v,s,l));
}
void LiteralTable::addLiteral(long int v, int s, int l,string sym){
  lits.push_back(Literal(v,s,l,sym));
}

void LiteralTable::print(){
 // cout<<"LITERAL TABLE: "<<endl;
  for(int i=0;i<lits.size();i++){
 //   cout<<lits[i].value<<endl;
  }
}
void LiteralTable::make(int start)&{
  int curr=start;
  for(Literal& l:lits){
    l.location=curr;
    curr+=l.size;
  }
}

long LiteralTable::getLocation(long value){
  for(Literal l:lits){
    if(l.value==value && l.symbol=="") {;return l.location;}
  }
  return 0;
}
long LiteralTable::getLocation(string s){
  for(Literal l:lits){
    if(!strcmp(l.symbol.c_str(),s.c_str())) {return l.location;}
  }
  return 0;
}

int LiteralTable::getFullSize(){
  int size=0;
  for(Literal& l:lits){
    size+=l.size;
  }
  return size;
}