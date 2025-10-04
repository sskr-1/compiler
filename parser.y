%{
#include <iostream>
#include <vector>
#include <string>
#include "ast.h"

// Global line number for error reporting
int line_num = 1;

// Forward declarations
void yyerror(const char* msg);
int yylex(void);

// Global variables for AST
Program* program;

// Include AST node definitions
// This will be defined in ast.h
%}

%union {
    int int_val;
    float float_val;
    std::string* str_val;
    Program* program;
    Function* function;
    Statement* statement;
    Expression* expression;
    Declaration* declaration;
    std::vector<Declaration*>* declaration_list;
    std::vector<Statement*>* statement_list;
    std::vector<Expression*>* expression_list;
    Type* type;
    std::vector<std::string*>* param_list;
}

/* Token definitions */
%token <str_val> IDENTIFIER
%token <int_val> INT_CONST
%token <float_val> FLOAT_CONST
%token <str_val> STRING_CONST

%token INT FLOAT DOUBLE CHAR VOID
%token IF ELSE WHILE FOR RETURN EXTERN
%token PLUS MINUS MULTIPLY DIVIDE MODULO
%token ASSIGN EQUAL NOT_EQUAL LESS GREATER LESS_EQUAL GREATER_EQUAL
%token LOGICAL_AND LOGICAL_OR LOGICAL_NOT
%token BITWISE_AND BITWISE_OR BITWISE_XOR BITWISE_NOT
%token SHIFT_LEFT SHIFT_RIGHT
%token LPAREN RPAREN LBRACE RBRACE LBRACKET RBRACKET
%token COMMA SEMICOLON COLON

/* Non-terminals */
%type <program> program
%type <function> function_declaration function_definition
%type <declaration_list> declaration_list
%type <statement_list> statement_list
%type <statement> statement
%type <expression> expression
%type <expression_list> expression_list argument_list
%type <declaration> declaration
%type <type> type
%type <param_list> parameter_list

/* Precedence and associativity */
%left LOGICAL_OR
%left LOGICAL_AND
%left BITWISE_OR
%left BITWISE_XOR
%left BITWISE_AND
%left EQUAL NOT_EQUAL
%left LESS GREATER LESS_EQUAL GREATER_EQUAL
%left SHIFT_LEFT SHIFT_RIGHT
%left PLUS MINUS
%left MULTIPLY DIVIDE MODULO
%right LOGICAL_NOT BITWISE_NOT UNARY_MINUS
%left LPAREN RPAREN LBRACKET RBRACKET

%%

program:
    declaration_list
    {
        program = new Program($1);
    }
    ;

declaration_list:
    declaration
    {
        $$ = new std::vector<Declaration*>();
        $$->push_back($1);
    }
    | declaration_list declaration
    {
        $1->push_back($2);
        $$ = $1;
    }
    ;

declaration:
    function_declaration SEMICOLON
    {
        $$ = $1;
    }
    | function_definition
    {
        $$ = $1;
    }
    | type IDENTIFIER SEMICOLON
    {
        $$ = new VariableDeclaration($1, *$2, line_num);
        delete $2;
    }
    | type IDENTIFIER ASSIGN expression SEMICOLON
    {
        $$ = new VariableDeclaration($1, *$2, $4, line_num);
        delete $2;
    }
    | EXTERN type IDENTIFIER LPAREN parameter_list RPAREN SEMICOLON
    {
        $$ = new ExternFunctionDeclaration($2, *$3, $5, line_num);
        delete $3;
    }
    ;

function_declaration:
    type IDENTIFIER LPAREN parameter_list RPAREN
    {
        $$ = new FunctionDeclaration($1, *$2, $4, line_num);
        delete $2;
    }
    ;

function_definition:
    type IDENTIFIER LPAREN parameter_list RPAREN LBRACE statement_list RBRACE
    {
        $$ = new FunctionDefinition($1, *$2, $4, $7, line_num);
        delete $2;
    }
    ;

parameter_list:
    /* empty */
    {
        $$ = new std::vector<std::string*>();
    }
    | type IDENTIFIER
    {
        $$ = new std::vector<std::string*>();
        $$->push_back($2);
    }
    | parameter_list COMMA type IDENTIFIER
    {
        $1->push_back($4);
        $$ = $1;
    }
    ;

type:
    INT
    {
        $$ = new Type("int", line_num);
    }
    | FLOAT
    {
        $$ = new Type("float", line_num);
    }
    | DOUBLE
    {
        $$ = new Type("double", line_num);
    }
    | CHAR
    {
        $$ = new Type("char", line_num);
    }
    | VOID
    {
        $$ = new Type("void", line_num);
    }
    ;

statement_list:
    statement
    {
        $$ = new std::vector<Statement*>();
        $$->push_back($1);
    }
    | statement_list statement
    {
        $1->push_back($2);
        $$ = $1;
    }
    ;

statement:
    declaration
    {
        $$ = $1;
    }
    | expression SEMICOLON
    {
        $$ = new ExpressionStatement($1, line_num);
    }
    | RETURN expression SEMICOLON
    {
        $$ = new ReturnStatement($2, line_num);
    }
    | RETURN SEMICOLON
    {
        $$ = new ReturnStatement(nullptr, line_num);
    }
    | IF LPAREN expression RPAREN statement
    {
        $$ = new IfStatement($3, $5, nullptr, line_num);
    }
    | IF LPAREN expression RPAREN statement ELSE statement
    {
        $$ = new IfStatement($3, $5, $7, line_num);
    }
    | WHILE LPAREN expression RPAREN statement
    {
        $$ = new WhileStatement($3, $5, line_num);
    }
    | FOR LPAREN expression SEMICOLON expression SEMICOLON expression RPAREN statement
    {
        $$ = new ForStatement($3, $5, $7, $9, line_num);
    }
    | LBRACE statement_list RBRACE
    {
        $$ = new BlockStatement($2, line_num);
    }
    ;

expression:
    INT_CONST
    {
        $$ = new IntegerLiteral($1, line_num);
    }
    | FLOAT_CONST
    {
        $$ = new FloatLiteral($1, line_num);
    }
    | STRING_CONST
    {
        $$ = new StringLiteral(*$1, line_num);
        delete $1;
    }
    | IDENTIFIER
    {
        $$ = new VariableExpression(*$1, line_num);
        delete $1;
    }
    | IDENTIFIER LPAREN argument_list RPAREN
    {
        $$ = new FunctionCallExpression(*$1, $3, line_num);
        delete $1;
    }
    | expression PLUS expression
    {
        $$ = new BinaryOpExpression($1, "+", $3, line_num);
    }
    | expression MINUS expression
    {
        $$ = new BinaryOpExpression($1, "-", $3, line_num);
    }
    | expression MULTIPLY expression
    {
        $$ = new BinaryOpExpression($1, "*", $3, line_num);
    }
    | expression DIVIDE expression
    {
        $$ = new BinaryOpExpression($1, "/", $3, line_num);
    }
    | expression MODULO expression
    {
        $$ = new BinaryOpExpression($1, "%", $3, line_num);
    }
    | expression EQUAL expression
    {
        $$ = new BinaryOpExpression($1, "==", $3, line_num);
    }
    | expression NOT_EQUAL expression
    {
        $$ = new BinaryOpExpression($1, "!=", $3, line_num);
    }
    | expression LESS expression
    {
        $$ = new BinaryOpExpression($1, "<", $3, line_num);
    }
    | expression GREATER expression
    {
        $$ = new BinaryOpExpression($1, ">", $3, line_num);
    }
    | expression LESS_EQUAL expression
    {
        $$ = new BinaryOpExpression($1, "<=", $3, line_num);
    }
    | expression GREATER_EQUAL expression
    {
        $$ = new BinaryOpExpression($1, ">=", $3, line_num);
    }
    | expression LOGICAL_AND expression
    {
        $$ = new BinaryOpExpression($1, "&&", $3, line_num);
    }
    | expression LOGICAL_OR expression
    {
        $$ = new BinaryOpExpression($1, "||", $3, line_num);
    }
    | expression BITWISE_AND expression
    {
        $$ = new BinaryOpExpression($1, "&", $3, line_num);
    }
    | expression BITWISE_OR expression
    {
        $$ = new BinaryOpExpression($1, "|", $3, line_num);
    }
    | expression BITWISE_XOR expression
    {
        $$ = new BinaryOpExpression($1, "^", $3, line_num);
    }
    | expression SHIFT_LEFT expression
    {
        $$ = new BinaryOpExpression($1, "<<", $3, line_num);
    }
    | expression SHIFT_RIGHT expression
    {
        $$ = new BinaryOpExpression($1, ">>", $3, line_num);
    }
    | LOGICAL_NOT expression
    {
        $$ = new UnaryOpExpression("!", $2, line_num);
    }
    | BITWISE_NOT expression
    {
        $$ = new UnaryOpExpression("~", $2, line_num);
    }
    | MINUS expression %prec UNARY_MINUS
    {
        $$ = new UnaryOpExpression("-", $2, line_num);
    }
    | LPAREN expression RPAREN
    {
        $$ = $2;
    }
    | expression ASSIGN expression
    {
        $$ = new AssignmentExpression($1, $3, line_num);
    }
    ;

argument_list:
    /* empty */
    {
        $$ = new std::vector<Expression*>();
    }
    | expression
    {
        $$ = new std::vector<Expression*>();
        $$->push_back($1);
    }
    | argument_list COMMA expression
    {
        $1->push_back($3);
        $$ = $1;
    }
    ;

%%

void yyerror(const char* msg) {
    std::cerr << "Parse error: " << msg << " at line " << line_num << std::endl;
}