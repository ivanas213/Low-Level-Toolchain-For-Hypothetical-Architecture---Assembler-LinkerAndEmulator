%{
#include "./inc/Emulator.hpp"
%}

%option c++
%option noyywrap

blank " "
hex " "[0-9A-Fa-f]{2}
address [0-9A-Fa-f]{8}": "

%%

{hex} {Emulator::putInBuffer(yytext);Emulator::setReadingHex();return 1;}
{blank} {return 0;}

{address} {Emulator::putInBuffer(yytext);Emulator::setReadingAddress();return 1;}
\n {Emulator::putInBuffer("new row");return 1000;}
<<EOF>> {Emulator::setEnd();return 0;}
. {Emulator::putInBuffer("unk");return 111;}

%%


