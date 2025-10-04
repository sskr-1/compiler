%{
#include <string>
#include <vector>
#include "ast_nodes.h"
#include "codegen.h"

extern int yylex();
extern int yylineno;
extern char* yytext;
extern FILE* yyin;

void yyerror(const char* s);
int yyparse();

// Global variables for AST
extern ASTNode* g_root;
extern CodeGenerator* g_codegen;
%}

%union {
    int int_val;
    float float_val;
    char char_val;
    char* string_val;
    ASTNode* node;
    std::vector<ASTNode*>* node_list;
}

%token <int_val> INTEGER_LITERAL
%token <float_val> FLOAT_LITERAL
%token <char_val> CHAR_LITERAL
%token <string_val> STRING_LITERAL IDENTIFIER

%token INT FLOAT CHAR VOID
%token IF ELSE WHILE FOR RETURN BREAK CONTINUE
%token TRUE FALSE
%token PLUS MINUS MULTIPLY DIVIDE MODULO
%token EQUAL NOT_EQUAL LESS_THAN GREATER_THAN LESS_EQUAL GREATER_EQUAL
%token AND OR NOT
%token ASSIGN PLUS_ASSIGN MINUS_ASSIGN MULTIPLY_ASSIGN DIVIDE_ASSIGN
%token INCREMENT DECREMENT
%token LPAREN RPAREN LBRACE RBRACE LBRACKET RBRACKET
%token SEMICOLON COMMA

%type <node> program function_declaration function_definition
%type <node> statement expression declaration
%type <node> if_statement while_statement for_statement
%type <node> return_statement break_statement continue_statement
%type <node> compound_statement expression_statement
%type <node> assignment_expression binary_expression unary_expression
%type <node> primary_expression function_call array_access
%type <node> variable_declaration parameter_list
%type <node_list> statement_list declaration_list parameter_list_items

%left OR
%left AND
%left EQUAL NOT_EQUAL LESS_THAN GREATER_THAN LESS_EQUAL GREATER_EQUAL
%left PLUS MINUS
%left MULTIPLY DIVIDE MODULO
%right NOT
%right ASSIGN PLUS_ASSIGN MINUS_ASSIGN MULTIPLY_ASSIGN DIVIDE_ASSIGN
%right INCREMENT DECREMENT

%%

program:
    declaration_list {
        g_root = new ProgramNode($1);
    }
    | /* empty */ {
        g_root = new ProgramNode(new std::vector<ASTNode*>());
    }
    ;

declaration_list:
    declaration_list declaration {
        $1->push_back($2);
        $$ = $1;
    }
    | declaration {
        $$ = new std::vector<ASTNode*>();
        $$->push_back($1);
    }
    ;

declaration:
    function_definition {
        $$ = $1;
    }
    | variable_declaration {
        $$ = $1;
    }
    ;

function_definition:
    type_specifier IDENTIFIER LPAREN parameter_list RPAREN compound_statement {
        $$ = new FunctionDefinitionNode($1, $2, $4, $6);
    }
    | type_specifier IDENTIFIER LPAREN RPAREN compound_statement {
        $$ = new FunctionDefinitionNode($1, $2, new std::vector<ASTNode*>(), $5);
    }
    ;

parameter_list:
    parameter_list_items {
        $$ = new ParameterListNode($1);
    }
    | /* empty */ {
        $$ = new ParameterListNode(new std::vector<ASTNode*>());
    }
    ;

parameter_list_items:
    parameter_list_items COMMA type_specifier IDENTIFIER {
        $1->push_back(new ParameterNode($3, $4));
        $$ = $1;
    }
    | type_specifier IDENTIFIER {
        $$ = new std::vector<ASTNode*>();
        $$->push_back(new ParameterNode($1, $2));
    }
    ;

variable_declaration:
    type_specifier IDENTIFIER SEMICOLON {
        $$ = new VariableDeclarationNode($1, $2, nullptr);
    }
    | type_specifier IDENTIFIER ASSIGN expression SEMICOLON {
        $$ = new VariableDeclarationNode($1, $2, $4);
    }
    ;

type_specifier:
    INT { $$ = new TypeNode("int"); }
    | FLOAT { $$ = new TypeNode("float"); }
    | CHAR { $$ = new TypeNode("char"); }
    | VOID { $$ = new TypeNode("void"); }
    ;

compound_statement:
    LBRACE statement_list RBRACE {
        $$ = new CompoundStatementNode($2);
    }
    | LBRACE RBRACE {
        $$ = new CompoundStatementNode(new std::vector<ASTNode*>());
    }
    ;

statement_list:
    statement_list statement {
        $1->push_back($2);
        $$ = $1;
    }
    | statement {
        $$ = new std::vector<ASTNode*>();
        $$->push_back($1);
    }
    ;

statement:
    expression_statement {
        $$ = $1;
    }
    | compound_statement {
        $$ = $1;
    }
    | if_statement {
        $$ = $1;
    }
    | while_statement {
        $$ = $1;
    }
    | for_statement {
        $$ = $1;
    }
    | return_statement {
        $$ = $1;
    }
    | break_statement {
        $$ = $1;
    }
    | continue_statement {
        $$ = $1;
    }
    | variable_declaration {
        $$ = $1;
    }
    ;

expression_statement:
    expression SEMICOLON {
        $$ = new ExpressionStatementNode($1);
    }
    | SEMICOLON {
        $$ = new ExpressionStatementNode(nullptr);
    }
    ;

if_statement:
    IF LPAREN expression RPAREN statement {
        $$ = new IfStatementNode($3, $5, nullptr);
    }
    | IF LPAREN expression RPAREN statement ELSE statement {
        $$ = new IfStatementNode($3, $5, $7);
    }
    ;

while_statement:
    WHILE LPAREN expression RPAREN statement {
        $$ = new WhileStatementNode($3, $5);
    }
    ;

for_statement:
    FOR LPAREN expression_statement expression_statement expression RPAREN statement {
        $$ = new ForStatementNode($3, $4, $5, $7);
    }
    ;

return_statement:
    RETURN expression SEMICOLON {
        $$ = new ReturnStatementNode($2);
    }
    | RETURN SEMICOLON {
        $$ = new ReturnStatementNode(nullptr);
    }
    ;

break_statement:
    BREAK SEMICOLON {
        $$ = new BreakStatementNode();
    }
    ;

continue_statement:
    CONTINUE SEMICOLON {
        $$ = new ContinueStatementNode();
    }
    ;

expression:
    assignment_expression {
        $$ = $1;
    }
    ;

assignment_expression:
    binary_expression {
        $$ = $1;
    }
    | unary_expression ASSIGN assignment_expression {
        $$ = new AssignmentNode($1, $3);
    }
    | unary_expression PLUS_ASSIGN assignment_expression {
        $$ = new AssignmentNode($1, new BinaryExpressionNode("+", $1, $3));
    }
    | unary_expression MINUS_ASSIGN assignment_expression {
        $$ = new AssignmentNode($1, new BinaryExpressionNode("-", $1, $3));
    }
    | unary_expression MULTIPLY_ASSIGN assignment_expression {
        $$ = new AssignmentNode($1, new BinaryExpressionNode("*", $1, $3));
    }
    | unary_expression DIVIDE_ASSIGN assignment_expression {
        $$ = new AssignmentNode($1, new BinaryExpressionNode("/", $1, $3));
    }
    ;

binary_expression:
    binary_expression OR binary_expression {
        $$ = new BinaryExpressionNode("||", $1, $3);
    }
    | binary_expression AND binary_expression {
        $$ = new BinaryExpressionNode("&&", $1, $3);
    }
    | binary_expression EQUAL binary_expression {
        $$ = new BinaryExpressionNode("==", $1, $3);
    }
    | binary_expression NOT_EQUAL binary_expression {
        $$ = new BinaryExpressionNode("!=", $1, $3);
    }
    | binary_expression LESS_THAN binary_expression {
        $$ = new BinaryExpressionNode("<", $1, $3);
    }
    | binary_expression GREATER_THAN binary_expression {
        $$ = new BinaryExpressionNode(">", $1, $3);
    }
    | binary_expression LESS_EQUAL binary_expression {
        $$ = new BinaryExpressionNode("<=", $1, $3);
    }
    | binary_expression GREATER_EQUAL binary_expression {
        $$ = new BinaryExpressionNode(">=", $1, $3);
    }
    | binary_expression PLUS binary_expression {
        $$ = new BinaryExpressionNode("+", $1, $3);
    }
    | binary_expression MINUS binary_expression {
        $$ = new BinaryExpressionNode("-", $1, $3);
    }
    | binary_expression MULTIPLY binary_expression {
        $$ = new BinaryExpressionNode("*", $1, $3);
    }
    | binary_expression DIVIDE binary_expression {
        $$ = new BinaryExpressionNode("/", $1, $3);
    }
    | binary_expression MODULO binary_expression {
        $$ = new BinaryExpressionNode("%", $1, $3);
    }
    | unary_expression {
        $$ = $1;
    }
    ;

unary_expression:
    primary_expression {
        $$ = $1;
    }
    | NOT unary_expression {
        $$ = new UnaryExpressionNode("!", $2);
    }
    | MINUS unary_expression {
        $$ = new UnaryExpressionNode("-", $2);
    }
    | PLUS unary_expression {
        $$ = new UnaryExpressionNode("+", $2);
    }
    | INCREMENT unary_expression {
        $$ = new UnaryExpressionNode("++", $2);
    }
    | DECREMENT unary_expression {
        $$ = new UnaryExpressionNode("--", $2);
    }
    | unary_expression INCREMENT {
        $$ = new UnaryExpressionNode("++", $1, true);
    }
    | unary_expression DECREMENT {
        $$ = new UnaryExpressionNode("--", $1, true);
    }
    ;

primary_expression:
    IDENTIFIER {
        $$ = new IdentifierNode($1);
    }
    | INTEGER_LITERAL {
        $$ = new IntegerLiteralNode($1);
    }
    | FLOAT_LITERAL {
        $$ = new FloatLiteralNode($1);
    }
    | CHAR_LITERAL {
        $$ = new CharLiteralNode($1);
    }
    | STRING_LITERAL {
        $$ = new StringLiteralNode($1);
    }
    | TRUE {
        $$ = new BooleanLiteralNode(true);
    }
    | FALSE {
        $$ = new BooleanLiteralNode(false);
    }
    | LPAREN expression RPAREN {
        $$ = $2;
    }
    | function_call {
        $$ = $1;
    }
    | array_access {
        $$ = $1;
    }
    ;

function_call:
    IDENTIFIER LPAREN RPAREN {
        $$ = new FunctionCallNode($1, new std::vector<ASTNode*>());
    }
    | IDENTIFIER LPAREN expression RPAREN {
        std::vector<ASTNode*>* args = new std::vector<ASTNode*>();
        args->push_back($3);
        $$ = new FunctionCallNode($1, args);
    }
    | IDENTIFIER LPAREN expression COMMA expression RPAREN {
        std::vector<ASTNode*>* args = new std::vector<ASTNode*>();
        args->push_back($3);
        args->push_back($5);
        $$ = new FunctionCallNode($1, args);
    }
    ;

array_access:
    IDENTIFIER LBRACKET expression RBRACKET {
        $$ = new ArrayAccessNode($1, $3);
    }
    ;

%%

void yyerror(const char* s) {
    fprintf(stderr, "Parse error at line %d: %s\n", yylineno, s);
}