flex -o FlexerEmulator.yy.cc ./L/Emulator.l
g++ -o emulator -I ./h ./src/Emulator.cpp ./src/EmulatorMain.cpp FlexerEmulator.yy.cc


  