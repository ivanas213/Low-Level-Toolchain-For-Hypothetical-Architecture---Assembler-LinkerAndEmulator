
#include "../inc/Assembler.hpp"
//#include <FlexLexer.h>
using namespace std;
int main(int argc,char* argv[]){
  //koje duzine treba da bude argv
  string input;
  string output;
  if(argc!=4){
    cout<<"Program terminated";
  }
  if(!strcmp(argv[1],"-o")){
  input=argv[3];
  output=argv[2];
  
  }else{
    cout<<"Program terminated";
  }
  

  Assembler a=Assembler::init(input,output);
  a.firstPass();
  a.mediumPass();
  a.secondPass();
}