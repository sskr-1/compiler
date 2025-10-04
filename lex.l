%{
#include <iostream>
#include <string>
#include "parser.tab.h"

// Store current line number for error reporting
extern int line_num;

// Forward declaration of error function
void yyerror(const char* msg);
%}

%option noyywrap
%option nounput
%option noinput

DIGIT    [0-9]
LETTER   [a-zA-Z_]
ID       {LETTER}({LETTER}|{DIGIT})*
INT_CONST   {DIGIT}+
FLOAT_CONST {DIGIT}+\.{DIGIT}+
STRING_CONST \"([^\"\\]|\\.)*\"
WHITESPACE [ \t]+
NEWLINE    \n

%%

{WHITESPACE}    { /* ignore whitespace */ }
{NEWLINE}       { line_num++; }

"int"           { return INT; }
"float"         { return FLOAT; }
"double"        { return DOUBLE; }
"char"          { return CHAR; }
"void"          { return VOID; }
"if"            { return IF; }
"else"          { return ELSE; }
"while"         { return WHILE; }
"for"           { return FOR; }
"return"        { return RETURN; }
"extern"        { return EXTERN; }

{ID}            {
                    yylval.str_val = new std::string(yytext);
                    return IDENTIFIER;
                }

{INT_CONST}     {
                    yylval.int_val = std::stoi(yytext);
                    return INT_CONST;
                }

{FLOAT_CONST}   {
                    yylval.float_val = std::stof(yytext);
                    return FLOAT_CONST;
                }

{STRING_CONST}  {
                    // Remove quotes from string
                    std::string str = yytext;
                    str = str.substr(1, str.length() - 2);
                    yylval.str_val = new std::string(str);
                    return STRING_CONST;
                }

"+"             { return PLUS; }
"-"             { return MINUS; }
"*"             { return MULTIPLY; }
"/"             { return DIVIDE; }
"%"             { return MODULO; }
"="             { return ASSIGN; }
"=="            { return EQUAL; }
"!="            { return NOT_EQUAL; }
"<"             { return LESS; }
">"             { return GREATER; }
"<="            { return LESS_EQUAL; }
">="            { return GREATER_EQUAL; }
"&&"            { return LOGICAL_AND; }
"||"            { return LOGICAL_OR; }
"!"             { return LOGICAL_NOT; }
"&"             { return BITWISE_AND; }
"|"             { return BITWISE_OR; }
"^"             { return BITWISE_XOR; }
"~"             { return BITWISE_NOT; }
"<<"            { return SHIFT_LEFT; }
">>"            { return SHIFT_RIGHT; }

"("             { return LPAREN; }
")"             { return RPAREN; }
"{"             { return LBRACE; }
"}"             { return RBRACE; }
"["             { return LBRACKET; }
"]"             { return RBRACKET; }
","             { return COMMA; }
";"             { return SEMICOLON; }
":"             { return COLON; }

.               {
                    std::cerr << "Lexical error: unexpected character '" << yytext << "' at line " << line_num << std::endl;
                    yyerror("unexpected character");
                }

%%