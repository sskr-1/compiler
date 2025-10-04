#pragma once

#include <memory>
#include <string>
#include <utility>
#include <vector>

struct Expr;
struct Stmt;
struct Function;
struct TranslationUnit;

using ExprPtr = std::unique_ptr<Expr>;
using StmtPtr = std::unique_ptr<Stmt>;
using FuncPtr = std::unique_ptr<Function>;
using TUPtr = std::unique_ptr<TranslationUnit>;

struct Expr {
  virtual ~Expr() = default;
};

struct NumberExpr : Expr {
  long long value;
  explicit NumberExpr(long long v) : value(v) {}
};

struct VariableExpr : Expr {
  std::string name;
  explicit VariableExpr(std::string n) : name(std::move(n)) {}
};

struct BinaryExpr : Expr {
  std::string op;
  ExprPtr lhs;
  ExprPtr rhs;
  BinaryExpr(std::string op, ExprPtr lhs, ExprPtr rhs)
      : op(std::move(op)), lhs(std::move(lhs)), rhs(std::move(rhs)) {}
};

struct CallExpr : Expr {
  std::string callee;
  std::vector<ExprPtr> args;
  CallExpr(std::string c, std::vector<ExprPtr> a)
      : callee(std::move(c)), args(std::move(a)) {}
};

struct UnaryExpr : Expr {
  std::string op;
  ExprPtr operand;
  UnaryExpr(std::string o, ExprPtr e) : op(std::move(o)), operand(std::move(e)) {}
};

struct AssignExpr : Expr {
  std::string name;
  ExprPtr value;
  AssignExpr(std::string n, ExprPtr v) : name(std::move(n)), value(std::move(v)) {}
};

struct Stmt {
  virtual ~Stmt() = default;
};

struct ReturnStmt : Stmt {
  ExprPtr value;
  explicit ReturnStmt(ExprPtr v) : value(std::move(v)) {}
};

struct ExprStmt : Stmt {
  ExprPtr expr;
  explicit ExprStmt(ExprPtr e) : expr(std::move(e)) {}
};

struct VarDeclStmt : Stmt {
  std::string name;
  ExprPtr init; // optional
  VarDeclStmt(std::string n, ExprPtr i) : name(std::move(n)), init(std::move(i)) {}
};

struct IfStmt : Stmt {
  ExprPtr cond;
  std::vector<StmtPtr> thenStmts;
  std::vector<StmtPtr> elseStmts; // may be empty
};

struct WhileStmt : Stmt {
  ExprPtr cond;
  std::vector<StmtPtr> body;
};

struct FunctionParam {
  std::string name;
};

struct Function {
  std::string name;
  std::vector<FunctionParam> params;
  std::vector<StmtPtr> body;
};

struct ExternDecl {
  std::string name;
  std::vector<FunctionParam> params;
};

struct TranslationUnit {
  std::vector<std::unique_ptr<ExternDecl>> externs;
  std::vector<FuncPtr> functions;
};
