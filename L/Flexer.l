%{
#include "./inc/Assembler.hpp"
%}

%option c++
%option noyywrap


blank " "
letter [a-zA-Z]{1}
digit [0-9]{1}
label ([a-zA-Z_][_a-zA-Z0-9]*)":"
symbol ([a-zA-Z_][_a-zA-Z0-9]*)(","){0,1}
hex_number [-]{0,1}"0x"{1}[0-9A-Fa-f]{1,8}
decimal_number [-]{0,1}[0-9]*
number ({hex_number}|{decimal_number})(","){0,1}
gpr (("%r"{1}[0-9]|("1"[0-5]){1})|"%sp"|"%pc")(,){0,1}
csr ("%status"|"%handler"|"%cause")(,){0,1}
operand (["$"]{0,1}{number})|(["$"]{0,1}{symbol})|(("%r"{1}[0-9]|("1"[0-5]){1})|"%sp"|"%pc")|("["(("%r"{1}[0-9]|("1"[0-5]){1})|"%sp"|"%pc")"]"[","]{0,1})|("["(("%r"{1}[0-9]|("1"[0-5]){1})|"%sp"|"%pc")"+"(([-]{0,1}"0x"{1}[0-9A-Fa-f]{1,8})|([-]{0,1}(0)|([1-9]+(0-9)*)))"]"[","]{0,1})|("["(("%r"{1}[0-9]|("1"[0-5]){1})|"%sp"|"%pc")"]")("["(("%r"{1}[0-9]|("1"[0-5]){1})|"%sp"|"%pc")"+"([a-zA-Z_][_a-zA-Z0-9]*)"]"[","]{0,1})|("["(("%r"{1}[0-9]|("1"[0-5]){1})|"%sp"|"%pc")" + "(([-]{0,1}"0x"{1}[0-9A-Fa-f]{1,8})|([-]{0,1}(0)|([1-9]+(0-9)*)))"]"[","]{0,1})
string "\""[ -~]*"\""

%%
"#" {if(!Assembler::isReadingComment()){Assembler::putInBuffer(yytext);Assembler::setComment();} return 1;}
(".global"|".extern"|".section"|".word"|".skip"|".ascii"|".equ"|".end") {if(!Assembler::isReadingComment()){Assembler::putInBuffer(yytext);if(Assembler::isNewLine())Assembler::setCurrentAssembling(Assembler::Type::directive);}return 1;}
("halt"|"int"|"iret"|"ret") {if(!Assembler::isReadingComment()){Assembler::putInBuffer(yytext);Assembler::setCurrentAssembling(Assembler::Type::command);Assembler::setCurrentCommandType(Assembler::CommandType::zero_arg);}return 1;}
("call"|"jmp"|"push"|"pop"|"not") {if(!Assembler::isReadingComment()){Assembler::putInBuffer(yytext);Assembler::setCurrentAssembling(Assembler::Type::command);Assembler::setCurrentCommandType(Assembler::CommandType::one_arg);}return 1;}
("xchg"|"add"|"sub"|"mul"|"div"|"and"|"or"|"xor"|"shl"|"shr"|"ld"|"st"|"csrrd"|"csrwr") {if(!Assembler::isReadingComment()){Assembler::putInBuffer(yytext);Assembler::setCurrentAssembling(Assembler::Type::command);Assembler::setCurrentCommandType(Assembler::CommandType::two_args);}return 1;}
("beq"|"bne"|"bgt") {if(!Assembler::isReadingComment()){Assembler::putInBuffer(yytext);Assembler::setCurrentCommandType(Assembler::CommandType::three_args);Assembler::setCurrentAssembling(Assembler::Type::command);}return 1;}
{label} {if(!Assembler::isReadingComment()){Assembler::putInBuffer(yytext);Assembler::setReadingLabel();Assembler::setCurrentAssembling(Assembler::Type::label);}return 1;}
{string} {if(!Assembler::isReadingComment()){Assembler::putInBuffer(yytext);Assembler::setReadingString();}return 1;}
{gpr} {if(!Assembler::isReadingComment()){Assembler::putInBuffer(yytext);Assembler::setReadingRegister();}return 1;}
{blank} {if(!Assembler::isReadingComment()){Assembler::putInBuffer("blank");}return 0;}
{symbol} {if(!Assembler::isReadingComment()){Assembler::putInBuffer(yytext);Assembler::setReadingSymbols();}return 1;}
{number} {if(!Assembler::isReadingComment()){Assembler::putInBuffer(yytext);Assembler::setReadingNumber();}return 1;}
{operand} {if(!Assembler::isReadingComment()){Assembler::putInBuffer(yytext);Assembler::setReadingOperand();}return 1;}
{csr} {if(!Assembler::isReadingComment()){Assembler::putInBuffer(yytext);Assembler::setReadingCSR();}return 1;}
\n {Assembler::goToNewLine();Assembler::resetComment();Assembler::putInBuffer("NOVI RED");return 1000;}
<<EOF>> {Assembler::setEnd(); return 1;}
. {Assembler::putInBuffer(yytext);if(!Assembler::isReadingComment())return 111;else return 1;}

%%

