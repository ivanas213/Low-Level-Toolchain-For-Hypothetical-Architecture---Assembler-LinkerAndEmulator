flex -o FlexerAssembler.yy.cc ./L/Flexer.l
g++ -o assembler -I ./h ./src/SectionTable.cpp ./src/SymbolTable.cpp ./src/LiteralTable.cpp ./src/RelocationTable.cpp ./src/Assembler.cpp ./src/main.cpp FlexerAssembler.yy.cc
