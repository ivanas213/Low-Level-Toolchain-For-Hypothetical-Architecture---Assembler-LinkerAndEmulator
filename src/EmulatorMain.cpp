#include "../inc/Emulator.hpp"

int main(int argc,char* argv[]){
  string input;
  //if(argc!=2){
   // cout<<"Program terminated"<<endl;
  //}
 // else{
    input=argv[1];
    Emulator emulator=Emulator::init(input);
    emulator.load();
    emulator.job();
    //emulator.print();
 // }
}
