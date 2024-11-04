%{
#include "./inc/Linker.hpp"
%}

%option c++
%option noyywrap

blank " "
decimal_number [-]{0,1}[0-9]*
hex_number [-]{0,1}[0-9A-Fa-f]{1,8}
symbol ([a-zA-Z_][_a-zA-Z0-9]*)
name "#."{symbol}[.]{0,1}{symbol}
number ({hex_number}|{decimal_number})

%%

{blank} {return 0;}
{name} {Linker::putInBuffer(yytext);Linker::setReadingName();return 1;}
{decimal_number} {Linker::putInBuffer(yytext);Linker::setReadingNumber();return 1;}
{hex_number} {Linker::putInBuffer(yytext);Linker::setReadingHexNumber();return 1;}
{symbol} {Linker::putInBuffer(yytext);Linker::setReadingSymbol();return 1;}
\n {return 1000;}
\t {return 0;}
<<EOF>> {Linker::setEnd(); return 1;}
. {Linker::putInBuffer(yytext);return 111;}






%%
