#include "../inc/SymbolTable.hpp"

void SymbolTable::insertNewSymbol(string naziv, int vrednost, int velicina, Type tip,Bind povezivanje, string sekcija){
  symbols.push_back(Symbol(getSize(),vrednost,velicina,tip,povezivanje,sekcija,naziv));
}
void SymbolTable::setGlobal(string name){
  for (int i=0;i<symbols.size();i++){
    if(symbols[i].name==name){
      symbols[i].bind=GLOBAL;
    }
  }
}
void SymbolTable::setLocal(string name){
  for (int i=0;i<symbols.size();i++){
    if(symbols[i].name==name){
      symbols[i].bind=LOCAL;
    }
  }
}
bool SymbolTable::isDeclared(string symbol){
    for (int i=0;i<symbols.size();i++){
      if(symbols[i].name==symbol){
        return true;
      }
    
    }
    return false;
}
bool SymbolTable::isDefined(string symbol){
    for (int i=0;i<symbols.size();i++){
      if(symbols[i].name==symbol){
        if(!strcmp(symbols[i].ndx.c_str(),"UND"))return false;
        return true;
      }
      
  }
  return false;
}
bool SymbolTable::haveBind(string name){
     for (int i=0;i<symbols.size();i++){
      if(symbols[i].name==name){
        if(symbols[i].bind==LOCAL) return false;
      }
      
     }
     return true;
}
void SymbolTable::define(string symbol,int location,string section){
  for (int i=0;i<symbols.size();i++){
      if(symbols[i].name==symbol){
        symbols[i].value=location;
        symbols[i].ndx=section;
        
      }
      
  }
}
int SymbolTable::getValue(string symbol){
  for(Symbol s:symbols){
    if(!strcmp(s.name.c_str(),symbol.c_str())){
      return s.value;
    }
  }
  return -1;
}
SymbolTable::Type SymbolTable::getType(string symbol){
  for(Symbol s:symbols){
    if(!strcmp(s.name.c_str(),symbol.c_str())){
      return s.type;
    }
  }
  return Type::FUNC;
}

SymbolTable::Bind SymbolTable::getBind(string symbol){
  for(Symbol s:symbols){
    if(!strcmp(s.name.c_str(),symbol.c_str())){
        return s.bind;  
    }
  }
  return Bind::NOBIND;
}

string SymbolTable::getSection(string symbol){
  for(Symbol s:symbols){
    if(!strcmp(s.name.c_str(),symbol.c_str())){
        return s.ndx;  
    }
  }
  return "";
}

void SymbolTable::print(ostream& os){
  ios init(NULL);
  init.copyfmt(os);
  os<<setw(4)<<"Num"<<"\t\t"<<setw(8)<<"Value   "<<"\t\t"<<setw(4)<<"Size"<<"\t\t"<<setw(7)<<" Type  "<<"\t\t"<<setw(4)<<"Bind"<<"\t\t"<<setw(10)<<"Section   "<<"\t\t"<<setw(4)<<"Name"<<endl;
  for(auto s:symbols){
   // if(!strcmp(s.ndx.c_str(),"no section")) s.ndx="UND";
    string type;
    if(s.type==SECTION) type="section";
    else if (s.type==OBJECT) type="object";
    else type="unk type";
    string bind;
    if(s.bind==LOCAL) bind="LOC";
    else bind="GLOB";
    int val=s.value;
    if(s.value==-1) val=0;
    os<<setw(4)<<s.index<<"\t\t"<<setw(8)<<std::hex<<setfill('0')<<val;
    os.copyfmt(init);

    os<<"\t\t"<<setw(4)<<s.size<<"\t\t"<<setw(7)<<type<<"\t\t"<<setw(4)<<bind<<"\t\t"<<setw(10)<<s.ndx<<"\t\t"<<setw(4)<<s.name<<endl;
    os.copyfmt(init);
  
  }
}

void SymbolTable::setExtern(string name){
  for (int i=0;i<symbols.size();i++){
    if(symbols[i].name==name){
      symbols[i].isExtern=true;
      symbols[i].ndx="UND";
      symbols[i].bind=GLOBAL;
    }
  }
}
bool SymbolTable::isExtern(string name){
  for (int i=0;i<symbols.size();i++){
    if(symbols[i].name==name){
      return symbols[i].isExtern;
    }
  }
  return false;
}

bool SymbolTable::checkForErrors(){
  for(Symbol s:symbols){
    if(!s.isExtern && s.value==-1) return false;
  }
  return true;
}

void SymbolTable::setSection(string name,string section){
  for(Symbol& s:symbols){
    if(!strcmp(name.c_str(),s.name.c_str())) s.ndx=section;
  }
}

SymbolTable::Symbol& SymbolTable::getSymbol(int i){
  for(int i=0;i<symbols.size();i++){
    if(symbols[i].index==i) return symbols[i];
  }
  return symbols[0];
}


void SymbolTable::processForLinker(){

  for(Symbol& s:symbols){
    
  }
}
/*
for(int i=0;i<sectionTable->getSize();i++){
      sectionTable->getSection(i).pool_start=next;
      for(int j=0;j<sectionTable->getSection(i).getRelocationTable().getSize();j++){
        sectionTable->getSection(i).getRelocationTable().relocations[j].move(next);
      }
      next+=sectionTable->getSection(i).getSize();

    }
*/

void SymbolTable::moveSymbol(int sym, int addr){
  for(Symbol& s:symbols){
    if(s.index==sym) {s.move(addr);return;}
  }
}
