%{
#include <string>
#include "parser.tab.h"
#include "ast_nodes.h"

// Function to create string tokens
extern int yylval;
%}

%option noyywrap
%option yylineno

%%

"int"           { return INT; }
"float"         { return FLOAT; }
"char"          { return CHAR; }
"void"          { return VOID; }
"if"            { return IF; }
"else"          { return ELSE; }
"while"         { return WHILE; }
"for"           { return FOR; }
"return"        { return RETURN; }
"break"         { return BREAK; }
"continue"      { return CONTINUE; }
"true"          { return TRUE; }
"false"         { return FALSE; }

[a-zA-Z_][a-zA-Z0-9_]* {
    yylval.string_val = strdup(yytext);
    return IDENTIFIER;
}

[0-9]+ {
    yylval.int_val = atoi(yytext);
    return INTEGER_LITERAL;
}

[0-9]+\.[0-9]+ {
    yylval.float_val = atof(yytext);
    return FLOAT_LITERAL;
}

'[^']' {
    yylval.char_val = yytext[1];
    return CHAR_LITERAL;
}

\"([^\"\\]|\\.)*\" {
    yylval.string_val = strdup(yytext);
    return STRING_LITERAL;
}

"+"             { return PLUS; }
"-"             { return MINUS; }
"*"             { return MULTIPLY; }
"/"             { return DIVIDE; }
"%"             { return MODULO; }
"=="            { return EQUAL; }
"!="            { return NOT_EQUAL; }
"<"             { return LESS_THAN; }
">"             { return GREATER_THAN; }
"<="            { return LESS_EQUAL; }
">="            { return GREATER_EQUAL; }
"&&"            { return AND; }
"||"            { return OR; }
"!"             { return NOT; }
"="             { return ASSIGN; }
"+="            { return PLUS_ASSIGN; }
"-="            { return MINUS_ASSIGN; }
"*="            { return MULTIPLY_ASSIGN; }
"/="            { return DIVIDE_ASSIGN; }
"++"            { return INCREMENT; }
"--"            { return DECREMENT; }

"("             { return LPAREN; }
")"             { return RPAREN; }
"{"             { return LBRACE; }
"}"             { return RBRACE; }
"["             { return LBRACKET; }
"]"             { return RBRACKET; }
";"             { return SEMICOLON; }
","             { return COMMA; }

[ \t\r]+        { /* ignore whitespace */ }
\n              { /* ignore newlines */ }
"//".*          { /* ignore single line comments */ }
"/*"([^*]|\*+[^*/])*\*+"/" { /* ignore multi-line comments */ }

.               { return yytext[0]; }

%%

int yywrap() {
    return 1;
}