#include "../inc/RelocationTable.hpp"



/*void RelocationTable::addRelocation(RelocationTable::Relocation r){
  relocations.push_back(r);
}*/
void RelocationTable::addRelocation(long int offset,string symbol,int addend){
   Relocation r(offset,symbol,addend);
   relocations.push_back(r); 
}
 
void RelocationTable::move(long address){
  for(Relocation& r:relocations){
    r.move(address);
  }
}