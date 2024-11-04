#include "../inc/SectionTable.hpp"

int SectionTable::next=0;

void SectionTable::addCode(int ind,string code){
  for(Section& s:sections){
    if(s.ind==ind){
      s.code.push_back(code);
      return;
    }
  }
 
}
/*SectionTable::Section& SectionTable::getSection(const string name) {
  Section* ss=new Section("no section");
  for(Section& s:sections){
    if(!strcmp(s.name.c_str(),name.c_str())){
      return s;
    }
  }
  return *ss;
}*/

SectionTable::Section& SectionTable::getSection(int ind){
  Section* ss=new Section("no section");
  for(Section& s:sections){
    if(s.ind==ind)return s;
  }
  return *ss;
}
SectionTable::Section& SectionTable::getSection(string name){
  Section* ss=new Section("no section");
  for(Section& s:sections){
    if(!strcmp(s.name.c_str(),name.c_str()))return s;
  }
  return *ss;
}


void SectionTable::start(){
  for(int i=0;i<sections.size();i++){
    sections[i].startPool();
  }
}


void SectionTable::addLiteral(int ind,int v, int s, int l){
  for(Section& ss:sections){
    if(ss.ind==ind){
     if( !ss.lits.exists(v))ss.lits.addLiteral(v,s,l);
     break;
    }
  }
}
void SectionTable::addLiteral(int ind,int v, int s, int l,string sym){
  for(Section& ss:sections){
    if(ss.ind==ind){
     if( !ss.lits.exists(sym)){    
     ss.lits.addLiteral(v,s,l,sym);
     }
     
     
    }
    

  }
}

void SectionTable::setSize(string name,int size){
  for(Section& ss:sections){
    if(!strcmp(ss.name.c_str(),name.c_str())){
      ss.setSize(size);
      break;
    }
  }
}
void SectionTable::addSize(string name,int size){
  for(Section& ss:sections){
    if(!strcmp(ss.name.c_str(),name.c_str())){
      ss.setSize(ss.size+size);
      break;
    }
  }
}

void SectionTable::setSize(int ind,int size){
 for(Section& ss:sections){
    if(ss.ind==ind){
      ss.setSize(size);
      break;
    }
  }
}


void SectionTable::print(ostream& os) const {
  /*
  os<<"NAME\tSIZE"<<endl;
  for(auto s:sections){
    os<<s.name<<"\t"<<s.size<<endl;
  }*/
  ios init(NULL);
  init.copyfmt(os);
  for(auto s:sections){
    os<<"#."<<s.name<<endl;
    int counter=0;
    for(int i=0;i<s.code.size();i++){
      os<<s.code[i]<<" ";
      if((i+1)%8==0) os<<endl;
      else if((i+1)%4==0) os<<" ";
      counter++;
    }
    if(s.code.size()%8!=0) os<<endl;
    os<<std::dec<<endl;
  }
  for(auto s:sections){

    if(s.relocationTable.relocations.size()>0){    
      os<<"#.rela."<<s.name<<endl;
      os<<setw(8)<<"Offset"<<"\t"<<setw(15)<<"Symbol"<<"\t"<<setw(3)<<"Addend"<<endl;
    }
    for(auto r:s.relocationTable.relocations){
      
      os<<setw(8)<<setfill('0')<<std::hex<<r.offset;

      os.copyfmt(init);

      os<<std::dec<<"\t"<<setw(15)<<r.symbol<<"\t"<<setw(3)<<std::hex<<r.addend<<std::dec<<endl;
      os.copyfmt(init);

    }
  }
  
  
}

/*void SectionTable::setStart(int ind,int start){
  for(Section& s:sections){
    if(s.ind==ind) {
      s.start=start;
      return;
    }
  }
}*/
bool SectionTable::exists(string name){
  for(Section& s:sections){
    if(!strcmp(s.name.c_str(),name.c_str())) return true;
  }
  return false;
}

void SectionTable::moveAfter(Section& s){
  int i;
  
  for(i=0;i<sections.size();i++){
    if(sections[i].ind==s.ind) break;
  }
  for(;i<sections.size();i++){
    sections[i].start=sections[i].start+s.getSize();
  }
}
void SectionTable::addRelocation(int n,int o, string s, int a){
  for(Section& ss:sections){
    if(ss.ind==n) ss.addRelocation(o,s,a);
  }
}


void SectionTable::move(int ind, int offset){
  for(Section& s:sections){
    if(s.ind==ind){
      for(RelocationTable::Relocation& r:s.getRelocations()){
        r.move(offset);
      }
      return;
    }
  }
}

void SectionTable::setStart(int i,int s){
  for(Section& ss:sections){
    if(ss.ind==i) {ss.setStart(s);return;}
  }
}
void SectionTable::processForLinker(int startAddress,vector<string> done){
  int next=startAddress;
  for(Section& s:sections){
    bool flag=true;
    for(auto str:done){
      if(!strcmp(s.name.c_str(),str.c_str())) {
        flag=false;
        break;
      }
    }
    if(flag){
      s.setStart(next);
      
      next+=s.getSize();
    }
  }
}


