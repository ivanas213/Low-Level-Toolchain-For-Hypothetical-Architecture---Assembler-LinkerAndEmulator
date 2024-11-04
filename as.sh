flex -o flexerAssembler.yy.cc ./L/Flexer.l;
g++ -o assembler -I ./h  ./src/SectionTable.cpp ./src/SymbolTable.cpp ./src/RelocationTable.cpp ./src/LiteralTable.cpp ./src/Assembler.cpp ./src/main.cpp flexerAssembler.yy.cc;


