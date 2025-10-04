#ifndef PARSER_TAB_H
#define PARSER_TAB_H

#include <string>

// Token definitions for the parser
typedef union {
    int int_val;
    float float_val;
    std::string* str_val;
    void* node; // For AST nodes
} YYSTYPE;

extern YYSTYPE yylval;

// Token constants
#define IDENTIFIER 258
#define INT_CONST 259
#define FLOAT_CONST 260
#define STRING_CONST 261

#define INT 262
#define FLOAT 263
#define DOUBLE 264
#define CHAR 265
#define VOID 266
#define IF 267
#define ELSE 268
#define WHILE 269
#define FOR 270
#define RETURN 271
#define EXTERN 272

#define PLUS 273
#define MINUS 274
#define MULTIPLY 275
#define DIVIDE 276
#define MODULO 277
#define ASSIGN 278
#define EQUAL 279
#define NOT_EQUAL 280
#define LESS 281
#define GREATER 282
#define LESS_EQUAL 283
#define GREATER_EQUAL 284
#define LOGICAL_AND 285
#define LOGICAL_OR 286
#define LOGICAL_NOT 287
#define BITWISE_AND 288
#define BITWISE_OR 289
#define BITWISE_XOR 290
#define BITWISE_NOT 291
#define SHIFT_LEFT 292
#define SHIFT_RIGHT 293

#define LPAREN 294
#define RPAREN 295
#define LBRACE 296
#define RBRACE 297
#define LBRACKET 298
#define RBRACKET 299
#define COMMA 300
#define SEMICOLON 301
#define COLON 302

#endif // PARSER_TAB_H