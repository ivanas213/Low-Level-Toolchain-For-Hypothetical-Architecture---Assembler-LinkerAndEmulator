
#include "../inc/Linker.hpp"
int main(int argc,char* argv[]){
  bool relocatable=false;
  bool hex=false;
  bool error=false;
  string out="";
  vector<string> input;
  bool options=true;
  vector<Linker::SectionAddress> sections;
  for(int i=1;i<argc;i++){
    if(options && !strcmp(argv[i],"-o")){
      i++;
      if(out.size()==0) out=argv[i];
      else {
        error=true;
        break;
      }
    }
    else if(options && !strcmp(argv[i],"-hex")){
      hex=true;
    }
    else if(options && !strcmp(argv[i],"-relocatable")){
      relocatable=true;
    }
    else if(options && argv[i][0]=='-'){
      string place=argv[i];
      string sub=place.substr(0,7);
      if(strcmp(sub.c_str(),"-place=")){
        error=true;
      }
      else{
        place.erase(0,7);
        string section="";
        string address="";
        bool firstPart=true;
        int i;
        for(i=0;i<place.size();i++){
          if(firstPart && place[i]!='@') section.push_back(place[i]);
          else if(firstPart) {
            firstPart=false;
          }
          else if(place[i]=='@') {error==true;break;}
          else address.push_back(place[i]);
        }
        if(firstPart || section.size()==0 || address.size()==0 ){
          error=true;
          break;
        }
        else sections.push_back(Linker::SectionAddress(section,address));

      }
    }
    else{
      if(options) options=false;
      string in=argv[i];
      input.push_back(in);
    }
  }
  if((!hex && !relocatable) || (hex && relocatable)) {error=true;cout<<"Greska:Mora biti izabrana tacno jedna od dve opcije hex i relocatable!"<<endl;}
  else if(out.size()==0 ) {error=true;cout<<"Greska:Nema izlaznog fajla!"<<endl;}
  else if(input.size()==0) {error=true;cout<<"Greska:Nema ulaznih fajlova!"<<endl;}
  else{
    //cout<<sections.size()<<endl;
    Linker r=Linker::init(input,out,sections,hex);
    r.link();
    
  }
}
