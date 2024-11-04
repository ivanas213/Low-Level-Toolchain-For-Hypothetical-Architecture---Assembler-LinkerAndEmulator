#ifndef _section_table_hpp_
#define _section_table_hpp_
#include <string>
#include "string.h"
#include <vector>
#include <iostream>
#include "LiteralTable.hpp"
#include "RelocationTable.hpp"
#include <iomanip>

using namespace std;
class SectionTable{
  public:
  static int next;
  struct Section
  {
    int ind;
    const string name;
    int size=0;
    vector<string> code;
    int pool_start=-1;
    unsigned int start=0;
    LiteralTable lits;
    RelocationTable relocationTable;
    Section(const string n):name(n),size(0),ind(next++){
      lits=*(new LiteralTable());
      relocationTable=*(new RelocationTable());
    }
    Section(const string n,int s):name(n),size(s),ind(next++){
      lits=*(new LiteralTable());
      relocationTable=*(new RelocationTable());
    }
    void startPool()&{pool_start=size;lits.make(pool_start);}
    void setSize(int sz){size=sz;}
    int getSize(){return size;}
    RelocationTable& getRelocationTable(){return relocationTable;}
    vector<RelocationTable::Relocation>& getRelocations(){return relocationTable.relocations;}
    int getNextFreeSpace(){return lits.getFullSize()+pool_start;}
    void addRelocation(int o,string s, int a){relocationTable.addRelocation(o,s,a);}
    void setStart(unsigned int s){start=s;}
    void addLiteral(int v, int s, int l,string sym){
      lits.addLiteral(v,s,l,sym);
    }

  };
  
  vector<Section> sections;
  
  SectionTable(){}
  //void addSection(Section section){sections.push_back(section);cout<<"ADDED   "<<section.name<<endl;}
  void addSection(string s){sections.push_back(Section(s));}
  void addSection(string s,int sz){sections.push_back(Section(s,sz));}
  void setSize(int ind,int size);
  void setSize(string name,int size);   
  void addSize(string name,int size);
  int getSize() const {return sections.size();}    
  void addCode(int ind,string code);
  //void addCode(string code);
  //Section& getSection(const string name) ;
  Section& getSection(int ind);
  Section& getSection(string name);

  //Section getSection(int i);
  //void print() const{for(auto s:sections)cout<<"IME:"<<s.name<<" VELICINA:  "<<s.size<<" LITERALI:"<< s.lits->getSize()<<endl;}
  void print(ostream& os) const;
  void start();
  void addLiteral(int ind,int v, int s, int l);  
  void addLiteral(int ind,int v, int s, int l,string sym);
  void setStart(int ind,int start);
  bool exists(string name);
  void moveAfter(Section& s);
  void addRelocation(int n,int o, string s, int a);
  void move(int ind, int offset);
 // void setStart(int i,int s);
  void processForLinker(int startAddress,vector<string> done);
};

#endif