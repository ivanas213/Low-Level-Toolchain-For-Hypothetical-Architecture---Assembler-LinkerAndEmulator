flex -o FlexerLinker.yy.cc ./L/FlexerLinker.l
g++ -o linker -I ./h ./src/SectionTable.cpp ./src/SymbolTable.cpp ./src/LiteralTable.cpp ./src/RelocationTable.cpp ./src/Linker.cpp ./src/LinkerMain.cpp FlexerLinker.yy.cc
