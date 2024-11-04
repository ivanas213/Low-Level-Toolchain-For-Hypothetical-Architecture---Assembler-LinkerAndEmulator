#include "../inc/Linker.hpp"
#include <FlexLexer.h>

bool Linker::readingName=false;
bool Linker::readingNumber=false;
bool Linker::readingHexNumber=false;
bool Linker::readingSymbol=false;
bool Linker::end=false;
bool Linker::error;
bool Linker::hex;

int Linker::nextIndex=0;
int Linker::currentSection=0;

int Linker::startAddress=0;
int Linker::nextStart=0;

vector<string> Linker::input_files;

string Linker::output;

SymbolTable* Linker::symbolTable;
SectionTable* Linker::sectionTable;

FlexLexer* Linker::flexer;

Linker* Linker::linker;

vector<Linker::SectionAddress> Linker::sectionAddress;

const char* Linker::buffer;


Linker Linker::init(vector<string> input, string output,vector<SectionAddress> sa,bool hex){
  if(linker==nullptr){
    linker=new Linker(input,output,sa,hex);
  }
  return *linker;
}

Linker::Linker(vector<string> in,string out,vector<SectionAddress> sa,bool h){
  input_files=in;
  output=out;
  sectionAddress=sa;
  hex=h;
  symbolTable=new SymbolTable();
  sectionTable=new SectionTable();
}

int Linker::pickNext(){
  int p=0;
  while(p==0){
     p=flexer->yylex();
  }
  return p;
}
int Linker::pickNextAndRow(){
  int p=0;
  while(p==0|| p==1000){
     p=flexer->yylex();
  }
  return p;
}
bool Linker::compareBuffer(const char* c){
  string b=buffer;
  if(!strcmp(b.c_str(),c)) return true;
  return false;
}

int Linker::convertHex8ToNumber(string hex){
  int mul=0;
  int ret=0;
  while(hex.size()>0){
    char c=hex.back();
    hex.pop_back();
    int num;
    if(c>='0' && c<='9')num=0+c-'0';
    else num=10+c-'a';
    ret+=num*pow(16,mul++);
    
  }
  return ret;
}
unsigned int Linker::convertHex8ToNumber(string hex,bool b){
  int mul=0;
  unsigned int ret=0;
  while(hex.size()>0){
    char c=hex.back();
    hex.pop_back();
    int num;
    if(c>='0' && c<='9')num=0+c-'0';
    else num=10+c-'a';
    ret+=num*pow(16,mul++);
    
  }
  return ret;
}
char Linker::getHexChar(int number){
     if(number>=0 && number<=9) return '0'+number;
     else return 'A'+number-10;
  
}
void Linker::link(){
  for(string s:input_files){
    end=false;
    ifstream input_stream;
    input_stream.open(s,ios::in);
    if(!input_stream){
      cout<<"Nema ulaznog fajla";
    }
    flexer = new yyFlexLexer(&input_stream);
    int pick=pickNext();
    error=!compareBuffer("#.symtab");
    if(error) break;
    pick=pickNextAndRow();
    error=!compareBuffer("Num");
    if(error) break;
    pick=pickNext();
    error=!compareBuffer("Value");
    if(error) break;
    pick=pickNext();
    error=!compareBuffer("Size");
    if(error) break;
    pick=pickNext();
    error=!compareBuffer("Type");
    if(error) break;
    pick=pickNext();
    error=!compareBuffer("Bind");
    if(error) break;
    pick=pickNext();
    error=!compareBuffer("Section");
    if(error) break;
    pick=pickNext();
    error=!compareBuffer("Name");
    readingName=false;
    readingSymbol=false;

    if(error) break;
    while(true){
      int val;
      int size;
      SymbolTable::Type type;
      SymbolTable::Bind bind;
      string section;
      string name;
      //prvo citamo index ali to zanemarujemo jer mi dodeljujemo sami ovde index
      pick=pickNextAndRow();
      if(!readingNumber){
        if(readingName) break;
        else{
         error=true;
         break;
        }
      }
      
      readingNumber=false;
      //zatim citamo vrednost
      pick=pickNext();
        string sss=buffer;
        val=convertHex8ToNumber(sss);

      readingHexNumber=false;
      readingNumber=false;
      //zatim citamo velicinu
      pick=pickNext();
      if(!readingNumber){
        error=true;
        break;
      }
      readingNumber=false;
      string s=buffer;
      size=stoi(s);
      //zatim citamo tip
      pick=pickNext();
      if(!readingSymbol){
        error=true;
        break;
      }
      readingSymbol=false;
      bool object=compareBuffer("object");
      if(object) type=SymbolTable::OBJECT;
      else type=SymbolTable::Type::SECTION;
      //zatim citamo povezivanje
      pick=pickNext();
      if(!readingSymbol){
        error=true;
        break;
      }
      readingSymbol=false;
      bool global=compareBuffer("GLOB");
      if(global) bind=SymbolTable::GLOBAL;
      else bind=SymbolTable::LOCAL;
      //zatim citamo sekciju
      pick=pickNext();
      if(!readingSymbol){
        error=true;
        break;
      }
      readingSymbol=false;
      section=buffer;
      //zatim na kraju citamo ime
      pick=pickNext();
      if(!readingSymbol){
        error=true;
        break;
      }
      readingSymbol=false;
      name=buffer;
      SectionTable::Section sect=sectionTable->getSection(section);
      int toAdd=0;
      if(!strcmp(sect.name.c_str(),"no section")) toAdd=0;
      else toAdd=sect.getSize();
      if(bind==SymbolTable::GLOBAL){
        if(!symbolTable->isDeclared(name)){
          int a=val+toAdd;
          symbolTable->insertNewSymbol(name,a,size,type,bind,section);
        }
        else{
          string s=symbolTable->getSection(name);
          if(strcmp(s.c_str(),"UND")  && strcmp(section.c_str(),"UND") && symbolTable->getType(name)!=SymbolTable::SECTION){
            error=true;
            cout<<"Simbol "<<name<<" je visestruko definisan!"<<endl;
          }
          //else if(strcmp(s.c_str(),"UND")  && strcmp(section.c_str(),"UND")) {}
          else if(!symbolTable->isDefined(name)) {
            int a=val+toAdd;
            symbolTable->define(name,a,section);

          }
        }
      }

    }
    readingHexNumber=false;
    readingNumber=false;
    readingSymbol=false;
    readingName=false;
    bool rel=false;
    unordered_map<string, vector<int>> umap;

    if(!error){
     

      while(!end && !error){
        int add=0;
        int location_counter=0;
        readingName=false;
        string name=buffer;
        name.erase(0,2);
        string sub=name.substr(0,5);
        bool exist=false;
        
        if(!rel && !strcmp(sub.c_str(),"rela.")){
          name.erase(0,5);
          currentSection=sectionTable->getSection(name).ind;
          rel=true;
        }
        if(!rel){
         
          if(!sectionTable->exists(name)){
            sectionTable->addSection(name);
            add=0;
            umap[name].push_back(add);
          }
          else {
            exist=true;
            add=sectionTable->getSection(name).getSize();
            umap[name].push_back(add);
          }
          currentSection=sectionTable->getSection(name).ind;
          while(!end && !error){
            if(readingName) {readingName=false;break;}
            pick=pickNextAndRow();
            if(readingName || end){
              readingName=false;
              sectionTable->addSize(sectionTable->getSection(currentSection).name,location_counter);
              break;
            }
            else {
              readingHexNumber=false;
              readingNumber=false;
              sectionTable->addCode(currentSection,buffer);
              location_counter++;
            }
          }
          
        }
        else{
          sub=name.substr(0,5);
          if(!strcmp(sub.c_str(),"rela.")) name.erase(0,5);
          currentSection=sectionTable->getSection(name).ind;
          int offsetNum;
          string symbol;
          int addendNum;
          pick=pickNext();
          bool b=!compareBuffer("Offset");
          pick=pickNext();
          if(!b) break;
          pick=pickNext();
          error=!compareBuffer("Symbol");
          if(error) break;
          pick=pickNext();
          error=!compareBuffer("Addend");
          if(error) break;
          readingSymbol=false;
          int add=umap[name][0];
          umap[name].erase(umap[name].begin());
          while(!end && !error){
            pick=pickNextAndRow();
            if(readingName){readingName=false; break;}
            if(end) break;
            offsetNum=convertHex8ToNumber(buffer);
            pick=pickNext();
            symbol=buffer;
            pick=pickNext();
            addendNum=convertHex8ToNumber(buffer);           
            sectionTable->addRelocation(currentSection,offsetNum+add,symbol,addendNum);
          }
        }
      }
    }
    
   
    
   
  }
  if(!noUndefined()) {
    error=true;
    cout<<"Nisu svi simboli definisani"<<endl;
  }
  else{
    process();
    relocations();

    //symbolTable->print(cout);
   // sectionTable->print(cout);
    print();
  }
}

void Linker::print(){

  ofstream out_stream(output,ios::out);
  //out<<"----Symbol Table----"<<endl;
  if(!hex){
    out_stream<<"#.symtab"<<endl;
    symbolTable->print(out_stream);
    sectionTable->print(out_stream);
  }
  else{
    for(auto& s:sectionTable->sections){
      int addr=s.start;
      int i;
      for(i=0;i<s.code.size();i++){
        if((i%8)==0) {
          out_stream<<std::hex<<setw(8)<<setfill('0')<<addr<<": ";
        }
        addr++;
        out_stream<<" "<<s.code[i];
        if((i%8)==7) out_stream<<endl;
      }
      if((i%8)!=0) out_stream<<endl;

    }
  }
  

}
void Linker::process(){
  vector<string> done;
  if(!hex){
    sectionTable->processForLinker(0,done);
    for(SymbolTable::Symbol& s:symbolTable->symbols){
    string sec=symbolTable->getSection(s.name);
    SectionTable::Section section=sectionTable->getSection(sec);
    symbolTable->moveSymbol(s.index,section.start);
  }
  }
  else{
    struct StartEnd{
      unsigned int start;
      unsigned int end;
      StartEnd(int s,int e):start(s),end(e){}
     
    };
    int biggest=0;
    vector<StartEnd> startend;
    for(auto& sa:sectionAddress){
        string sec=sa.section;
        SectionTable::Section section=sectionTable->getSection(sec);
        if(!strcmp(section.name.c_str(),"no section")) {error=true;cout<<"Greska:Nepostojeca sekcija je uneta!"<<endl;break;}
        sa.address.erase(0,2);
        unsigned int addr=convertHex8ToNumber(sa.address,true);
        section.setStart(addr);
        int endd;
        bool error=false;
        for(auto se:startend){
          int start=addr;
          endd=addr+section.getSize();
          if((start>se.start && se.end>start)||(start<se.start && se.start<end)) {
            error=true;
            break;
          }
        }
        if(error){
          cout<<"Greska:Nije moguce ovako rasporediti sekcije"<<endl;
          break;
        }
        else{
          startend.push_back(StartEnd(addr,addr+section.getSize()));
          if(addr+section.getSize()>biggest) biggest=addr+section.getSize();
          done.push_back(sec);
          sectionTable->setStart(section.ind,addr);
        }
    }
    sectionTable->processForLinker(biggest,done);
    for(SymbolTable::Symbol& s:symbolTable->symbols){
    string sec=symbolTable->getSection(s.name);
    SectionTable::Section section=sectionTable->getSection(sec);
    symbolTable->moveSymbol(s.index,section.start);
    }

  }

}

void Linker::relocations(){
  for(SectionTable::Section& s:sectionTable->sections){
    for(RelocationTable::Relocation& r:s.relocationTable.relocations){
      unsigned  start=r.offset;
      unsigned int val=symbolTable->getValue(r.symbol)+r.addend;
      string str=getHex(val,8);
      for(int i=0;i<4;i++){
        string sub=str.substr(0,2);
        str.erase(0,2);
        s.code[start+i]=sub;
      }
    }
  }
}

string Linker::getHex(int number,int sz){
    string ret="";
    for(int i=0;i<sz/2;i++){
      int num=number%16;
      string s="";
      string ch="";
      ch.push_back(getHexChar(num));
      s.insert(0,ch);
      ch.erase(0,1);
      number=number>>4;
      num=number%16;
      ch.push_back(getHexChar(num));
      s.insert(0,ch);
      ret.append(s);
      number=number>>4;
    }
    return ret;
}
string Linker::getHex(unsigned int number,int sz){
    string ret="";
    for(int i=0;i<sz/2;i++){
      int num=number%16;
      string s="";
      string ch="";
      ch.push_back(getHexChar(num));
      s.insert(0,ch);
      ch.erase(0,1);
      number=number>>4;
      num=number%16;
      ch.push_back(getHexChar(num));
      s.insert(0,ch);
      ret.append(s);
      number=number>>4;
    }
    return ret;
}

bool Linker::noUndefined(){
  for(SymbolTable::Symbol& s:symbolTable->symbols){
    if(!strcmp(s.ndx.c_str(),"UND")) return false;
  }
  return true;
}
