#include "../inc/Emulator.hpp"
#include <FlexLexer.h>


bool Emulator::readingAddress=false;
bool Emulator::readingHex=false;
bool Emulator::end=false;
bool Emulator::error=false;
bool Emulator::interrupt=false;
bool Emulator::mem=false;

string Emulator::input;
ifstream Emulator::input_stream;

FlexLexer* Emulator::flexer;

std::tr1::unordered_map<uint32_t, uint8_t> Emulator::memory;

int32_t Emulator::registers[16];
int32_t Emulator::status;
int32_t Emulator::cause;
int32_t Emulator::handler;

Emulator* Emulator::emulator;

int Emulator::a=0;
int Emulator::b=0;
int Emulator::c=0;
int Emulator::d=0;

uint Emulator::operation;

const char* Emulator::buffer;



Emulator::Emulator(string in){
  input=in;
  input_stream.open(input,std::ios::binary);
  flexer = new yyFlexLexer(&input_stream);
 
  for(int i=0;i<16;i++){
    if(i==14) registers[i]=0xFFFFFF00;
    else if( i==15) registers[i]=0x40000000;
    else registers[i]=0;
  }
  status=0;
  cause=0;
  handler=0;
}

Emulator Emulator::init(string input){
  if(!emulator){
    emulator=new Emulator(input);
  }
  return *emulator;
}
char Emulator::getHexCharInverted(int number){
  switch(number){
    case 0:return 'F';
    case 1:return 'E';
    case 2:return 'D';
    case 3:return 'C';
    case 4:return 'B';
    case 5:return 'A';
    case 6:return '9';
    case 7:return '8';
    case 8:return '7';
    case 9:return '6';
    case 10:return '5';
    case 11:return '4';
    case 12:return '3';
    case 13:return '2';
    case 14:return '1';
    case 15:return '0';
    default:return 'x';
  }
}
char Emulator::getHexChar(int number){
  switch(number){
    case 0:return '0';
    case 1:return '1';
    case 2:return '2';
    case 3:return '3';
    case 4:return '4';
    case 5:return '5';
    case 6:return '6';
    case 7:return '7';
    case 8:return '8';
    case 9:return '9';
    case 10:return 'A';
    case 11:return 'B';
    case 12:return 'C';
    case 13:return 'D';
    case 14:return 'E';
    case 15:return 'F';
    default:return 'x';
  }
}
void Emulator::parseNextInstruction(){
 // cout<<"PC from parse="<<hex<<registers[15]<<endl;
  int next=registers[15];
  registers[15]=registers[15]+4;
  operation=memory[next++];
  //operation=getHex(memory[next++],2);
 // cout<<hex<<operation<<" ";
  uint8_t s=memory[next++];
 // cout<<uint(s)<<" ";
  a=(s>>4)&0b00001111;
  b=s&0b00001111;
  s=memory[next++];
 // cout<<uint(s)<<" ";
  c=(s>>4)&0b00001111;
  d=s&0b00001111;
  s=memory[next];
 // cout<<uint(s)<<endl;
  d=(d<<8)+s;
  convertD;
  //cout<<"a="<<a<<" b="<<b<<" c="<<c<<" d="<<d<<endl;
}

string Emulator::getHex(int number,int sz){
    string ret="";
    if(number>=0){
    for(int i=0;i<sz;i++){
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
      
    }}
    else{
      number=0-number;
      bool add=true;
      for(int i=0;i<4;i++){
        int num=number%16;
        string s="";
        string ch="";
        char hx=getHexCharInverted(num);
        if(add){
          if(hx=='F') hx='0';
          else {hx++;add=false;}
        }
        ch.push_back(hx);
        
        s.insert(0,ch);
        ch.erase(0,1);
        number=number>>4;
        num=number%16;
        number=number>>4;

        hx=getHexCharInverted(num);
        if(add){
          if(hx=='F') hx='0';
          else {hx++;add=false;}
        }
        ch.push_back(hx);
        s.insert(0,ch);
        ret.append(s);
      }
    }
    return ret;
}
void Emulator::job(){
  end=false;
  while(!end && !error){     
    parseNextInstruction();
    if(operation==0x00){
      processHalt();
    }
    else if(operation==0x10){
      processSoftwareInterrupt();
    }
    else if(operation==0x20){
      processStartSubroutine();
    }
    else if(operation==0x21){
      processStartSubroutineMem();
    }
    else if(operation==0x30){
      processJump();
    }
    else if(operation==0x31){
      processBEQ();
    }
    else if(operation==0x32){
      processBNE();
    }
    else if(operation==0x33){
      processBGT();
    }
    else if(operation==0x38){
      processJumpMem();
    }
    else if(operation==0x39){
      processBEQMem();
    }
    else if(operation==0x3A){
      processBNEMem();
    }
    else if(operation==0x3B){
      processBGTMem();
    }
    else if(operation==0x40){
      processChange();
    }
    else if(operation==0x50){
      processAdd();
    }
    else if(operation==0x51){
      processSub();
    }
    else if(operation==0x52){
      processMul();
    }
    else if(operation==0x53){
      processDiv();
    }
    else if(operation==0x60){
      processNot();
    }
    else if(operation==0x61){
      processAnd();
    }
    else if(operation==0x62){
      processOr();
    }
    else if(operation==0x63){
      processXor();
    }
    else if(operation==0x70){
      processShiftLeft();
    }
    else if(operation==0x71){
      processShiftRight();
    }
    else if(operation==0x80){
      processStore();
    }
    else if(operation==0x81){
      processStoreAdd();
    }
    else if(operation==0x82){
      processStoreStore();
    }
    else if(operation==0x90){
      processLdGrpCsr();
    }
    else if(operation==0x91){
      processLdGprGpr();
    }
    else if(operation==0x92){
      processLdGprGprMem();
    }
    else if(operation==0x93){
      processLdGprMemAdd();
    }
    else if(operation==0x94){
      processCsrGrp();
    }
    else if(operation==0x95){
      processCsrCsrD();
    }
    else if(operation==0x96){
      processCsrMem();
    }
    else if(operation==0x97){
      processCsrMemAdd();
    }
    else{
      error=true;
    }
  }
  //if(error) cout<<registers[15]<<endl;
  //else cout<<"PC="<<std::hex<<registers[15]<<endl;
  print();
}

void Emulator::load(){
  int pick=flexer->yylex();
  while(!end){
    readingAddress=false;
    string b=buffer;

    b.pop_back();
    b.pop_back();
    uint32_t address=hexToUInt(b);

    int i=0;

    pick=flexer->yylex();

    while(readingHex){
      readingHex=false;
      string b=buffer;
      b.erase(0,1);
      memory[address+i]=convertStringByteToInt8(b);
      i++;
      pick=flexer->yylex();
      if(!readingHex) pick=flexer->yylex();

    }
  }
}

int Emulator::hexToInt(string hex){
  int mul=0;
  int ret=0;
  while(hex.size()>0){
    char c=hex.back();
    hex.pop_back();
    int num;
    if(c>='0' && c<='9')num=0+c-'0';
    else if(c>='A' && c<='F') num=10+c-'A';
    else num=10+c-'a';
    ret+=num*pow(16,mul++);
    
  }
  return ret;
}

unsigned int Emulator::hexToUInt(string hex){
  int mul=0;
  unsigned int ret=0;
  while(hex.size()>0){
    char c=hex.back();
    hex.pop_back();
    int num;
    if(c>='0' && c<='9')num=0+c-'0';
    else if(c>='A' && c<='F') num=10+c-'A';
    else num=10+c-'a';
    ret+=num*pow(16,mul++);
    
  }
  return ret;
}
void Emulator::processHalt(){
  if(!(a==0 && b==0 && c==0 && d==0)){error=true;return;}
  end=true;
}

void Emulator::processSoftwareInterrupt(){
  if(!(a==0 && b==0 && c==0 && d==0)){error=true;return;}

  interrupt=true;
  push(int32_t(status));
  push(int32_t(registers[15]));
  cause=4;
  cause<=4; 
  status=status&(~0x1);
  registers[15]=handler;
}
void Emulator::processStartSubroutine(){

  push(int32_t(registers[15]));
  registers[15]=registers[a]+registers[b]+d;
}

void Emulator::processStartSubroutineMem(){

  push(int32_t(registers[15]));
  registers[15]=getFromMemory(registers[a]+registers[b]+d);
}
void Emulator::processJump(){

  registers[15]=registers[a]+d;
}

void Emulator::processBEQ(){
 
  if(registers[b]==registers[c]) registers[15]=registers[a]+d;
}
void Emulator::processBNE(){

  if(registers[b]!=registers[c]) registers[15]=registers[a]+d;
}
void Emulator::processBGT(){

  if(registers[b]>registers[c]) registers[15]=registers[a]+d;
}
void Emulator::processJumpMem(){
  uint32_t r=registers[a];
  r+=d;

  registers[15]=getFromMemory(r);
}

void Emulator::processBEQMem(){
  uint32_t r=registers[a];
  r+=d;
  if(registers[b]==registers[c]) registers[15]=getFromMemory(r);
}
void Emulator::processBNEMem(){
  uint32_t r=registers[a];
  r+=d;
  if(registers[b]!=registers[c]) registers[15]=getFromMemory(r);
}
void Emulator::processBGTMem(){
  uint32_t r=registers[a];
  r+=d;

  if(registers[b]>registers[c]) registers[15]=getFromMemory(r);
}
void Emulator::processChange(){
  if(d!=0) {error=true;return;}
  uint32_t temp=registers[b];
  registers[b]=registers[c];
  registers[c]=temp;
}
void Emulator::processAdd(){
  if(d!=0) {error=true;return;}
  registers[a]=registers[b]+registers[c];
}
void Emulator::processSub(){
  if(d!=0) {error=true;return;}
  registers[a]=registers[b]-registers[c];
}
void Emulator::processMul(){
  if(d!=0) {error=true;return;}
  registers[a]=registers[b]*registers[c];
}
void Emulator::processDiv(){
  if(d!=0) {error=true;return;}
  registers[a]=registers[b]/registers[c];
}
void Emulator::processNot(){
  if(d!=0) {error=true;return;}
  registers[a]=~registers[b];
}
void Emulator::processAnd(){
  if(d!=0) {error=true;return;}
  registers[a]=registers[b]&registers[c];
}
void Emulator::processOr(){
  if(d!=0) {error=true;return;}
  registers[a]=registers[b]|registers[c];
}
void Emulator::processXor(){
  if(d!=0) {error=true;return;}
  registers[a]=registers[b]|registers[c];
}
void Emulator::processShiftLeft(){
  if(d!=0) {error=true;return;}
  registers[a]=registers[b]<<registers[c];
}
void Emulator::processShiftRight(){
  if(d!=0) {error=true;return;}
  registers[a]=registers[b]>>registers[c];
}
void Emulator::processStore(){
  putToMemory(registers[a+registers[b]+d],registers[c]);
}
void Emulator::processStoreStore(){
  putToMemory(getFromMemory(registers[a]+registers[b]+d),registers[c]);
}
void Emulator::processStoreAdd(){
  if(a==14 || a==15 || mem ){
    push(registers[c]);
  }
  else{
  registers[a]=registers[a]+d;
  putToMemory(registers[a],registers[c]);

  }
}
void Emulator::processLdGrpCsr(){
  if(b==0)
  registers[a]=status;
  else if(b==1)
  registers[a]=handler;
  else registers[a]=cause;
}
void Emulator::processLdGprGpr(){
  registers[a]=registers[b]+d;
}
void Emulator::processLdGprGprMem(){
  registers[a]=getFromMemory(registers[b]+registers[c]+d);
}
void Emulator::processLdGprMemAdd(){
  uint32_t r=registers[b];
  registers[a]=getFromMemory(r);
  r+=d;
  registers[b]=r;


}
void Emulator::processCsrGrp(){
  if(b==0)
  status=registers[b];
  else if(b==1)
  handler=registers[b];
  else cause=registers[b];
}
void Emulator::processCsrCsrD(){
  if(a==0){
    if(b==0) status=status|d;
    else if(b==1) status=handler|d;
    else status=cause|d;
  }
  else if(a==1){
    if(b==0) handler=status|d;
    else if(b==1) handler=handler|d;
    else handler=cause|d;
  }
  else {
    if(b==0) cause=status|d;
    else if(b==1) cause=handler|d;
    else cause=cause|d;
  }
}
void Emulator::processCsrMem(){
  if(b==0){
  status=getFromMemory(registers[b]+registers[c]+d);
  }
  else if(b==1)
  handler=getFromMemory(registers[b]+registers[c]+d);
  else cause=getFromMemory(registers[b]+registers[c]+d);

}
void Emulator::processCsrMemAdd(){
  if(b==0)
  status=getFromMemory(registers[b]);
  else if(b==1)
  handler=getFromMemory(registers[b]);
  else cause=getFromMemory(registers[b]);
  registers[b]=registers[b]+d;

}
void Emulator::push(uint32_t val){
  uint32_t r=registers[14];
  r-=4;
  for(int i=0;i<4;i++){
    uint8_t v=(val>>(i*8))&0xff;
    memory[r++]=v;
  }
  registers[14]=r-4;

}
void Emulator::push(int32_t val){
  uint32_t r=registers[14];
  r-=4;
  for(int i=0;i<4;i++){
    uint8_t v=(val>>(i*8))&0xff;
    
    memory[r++]=v;
  }
  registers[14]=r-4;
}
void Emulator::print(){
  cout<<"Emulated processor executed halt instruction"<<endl<<"Emulated processor state:"<<endl;
  cout<<"r0=0x"<<setw(8)<<std::hex<<setfill('0')<<hex<<registers[0]<<"\t";
  cout<<"r1=0x"<<setw(8)<<std::hex<<setfill('0')<<registers[1]<<"\t";
  cout<<"r2=0x"<<setw(8)<<std::hex<<setfill('0')<<registers[2]<<"\t";
  cout<<"r3=0x"<<setw(8)<<std::hex<<setfill('0')<<registers[3]<<"\n";
  cout<<"r4=0x"<<setw(8)<<std::hex<<setfill('0')<<registers[4]<<"\t";
  cout<<"r5=0x"<<setw(8)<<std::hex<<setfill('0')<<registers[5]<<"\t";
  cout<<"r6=0x"<<setw(8)<<std::hex<<setfill('0')<<registers[6]<<"\t";
  cout<<"r7=0x"<<setw(8)<<std::hex<<setfill('0')<<registers[7]<<"\n";
  cout<<"r8=0x"<<setw(8)<<std::hex<<setfill('0')<<registers[8]<<"\t";
  cout<<"r9=0x"<<setw(8)<<std::hex<<setfill('0')<<registers[9]<<"\t";
  cout<<"r10=0x"<<setw(8)<<std::hex<<setfill('0')<<registers[10]<<"\t";
  cout<<"r11=0x"<<setw(8)<<std::hex<<setfill('0')<<registers[11]<<"\n";
  cout<<"r12=0x"<<setw(8)<<std::hex<<setfill('0')<<registers[12]<<"\t";
  cout<<"r13=0x"<<setw(8)<<std::hex<<setfill('0')<<registers[13]<<"\t";
  cout<<"r14=0x"<<setw(8)<<std::hex<<setfill('0')<<registers[14]<<"\t";
  cout<<"r15=0x"<<setw(8)<<std::hex<<setfill('0')<<hex<<registers[15]<<"\n";
  //cout<<"status=0x"<<std::hex<<setfill('0')<<hex<<status<<"\n";
  //cout<<"handler=0x"<<std::hex<<setfill('0')<<hex<<handler<<"\n";
  //cout<<"cause=0x"<<std::hex<<setfill('0')<<hex<<cause<<"\n";

}
int Emulator::getIntFromHexChar(char c){
  if(c>='0' && c<='9') return c-'0';
  else return 10+c-'A';
}

void Emulator::convertD(){
  if(d>=2048){
    int ret=0;

    while(d>0){
    int os=d%16;
    ret=ret<<4;
    os=~os;
    ret+=os;
    d/=16;
    }
    d=-(ret+1);

  }
}

int8_t Emulator::convertStringByteToInt8(string b){
  int8_t ret=getIntFromHexChar(b[0])*16+getIntFromHexChar(b[1]);
  return ret;
}
int32_t Emulator::getFromMemory(int32_t address,bool b){
  int32_t next=address+3;
  int32_t ret=0;
  for(int i=0;i<4;i++){
    ret=ret<<8;
    ret+=memory[next--];
  }
  return ret;
}
uint32_t Emulator::getFromMemory(int32_t address){
  int32_t next=address+3;
  int32_t ret=0;
  for(int i=0;i<4;i++){
    ret=ret<<8;
    ret+=memory[next--];
  }
  return ret;
}
void Emulator::putToMemory(uint32_t address, uint32_t data){
  uint32_t next=address;
  int d=data;
  uint8_t data1=((d>>24)& 0xff);
  uint8_t data2=((d>>16)& 0xff);
  uint8_t data3=((d>>8)& 0xff);
  uint8_t data4=(d& 0xff);
  memory[address++]=data4;
  memory[address++]=data3;
  memory[address++]=data2;
  memory[address]=data1;
  
}
