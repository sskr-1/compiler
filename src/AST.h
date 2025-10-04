#pragma once

#include <cstdint>
#include <map>
#include <memory>
#include <optional>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

namespace llvm {
class Value;
}

namespace mini {

class CodeGenContext;

enum class BinaryOp {
  Add,
  Sub,
  Mul,
  Div,
  LT,
  GT,
  LE,
  GE,
  EQ,
  NE
};

class Expr {
 public:
  virtual ~Expr() = default;
  virtual llvm::Value* codegen(CodeGenContext& context) = 0;
};

class NumberExpr : public Expr {
 public:
  explicit NumberExpr(std::int64_t value) : value(value) {}
  llvm::Value* codegen(CodeGenContext& context) override;
 private:
  std::int64_t value;
};

class VariableExpr : public Expr {
 public:
  explicit VariableExpr(std::string name) : name(std::move(name)) {}
  llvm::Value* codegen(CodeGenContext& context) override;
  const std::string& getName() const { return name; }
 private:
  std::string name;
};

class AssignExpr : public Expr {
 public:
  AssignExpr(std::string variableName, std::unique_ptr<Expr> value)
      : variableName(std::move(variableName)), value(std::move(value)) {}
  llvm::Value* codegen(CodeGenContext& context) override;
 private:
  std::string variableName;
  std::unique_ptr<Expr> value;
};

class BinaryExpr : public Expr {
 public:
  BinaryExpr(BinaryOp op, std::unique_ptr<Expr> lhs, std::unique_ptr<Expr> rhs)
      : op(op), lhs(std::move(lhs)), rhs(std::move(rhs)) {}
  llvm::Value* codegen(CodeGenContext& context) override;
 private:
  BinaryOp op;
  std::unique_ptr<Expr> lhs;
  std::unique_ptr<Expr> rhs;
};

class CallExpr : public Expr {
 public:
  CallExpr(std::string callee, std::vector<std::unique_ptr<Expr>> args)
      : callee(std::move(callee)), args(std::move(args)) {}
  llvm::Value* codegen(CodeGenContext& context) override;
 private:
  std::string callee;
  std::vector<std::unique_ptr<Expr>> args;
};

class Stmt {
 public:
  virtual ~Stmt() = default;
  virtual void codegen(CodeGenContext& context) = 0;
};

class ExprStmt : public Stmt {
 public:
  explicit ExprStmt(std::unique_ptr<Expr> expr) : expr(std::move(expr)) {}
  void codegen(CodeGenContext& context) override;
 private:
  std::unique_ptr<Expr> expr;
};

class ReturnStmt : public Stmt {
 public:
  explicit ReturnStmt(std::unique_ptr<Expr> value) : value(std::move(value)) {}
  void codegen(CodeGenContext& context) override;
 private:
  std::unique_ptr<Expr> value;  // May be null for bare return
};

class VarDeclStmt : public Stmt {
 public:
  VarDeclStmt(std::string name, std::unique_ptr<Expr> init)
      : name(std::move(name)), init(std::move(init)) {}
  void codegen(CodeGenContext& context) override;
 private:
  std::string name;
  std::unique_ptr<Expr> init;  // May be null
};

class BlockStmt : public Stmt {
 public:
  BlockStmt() = default;
  void add(std::unique_ptr<Stmt> stmt) { statements.push_back(std::move(stmt)); }
  void codegen(CodeGenContext& context) override;
 private:
  std::vector<std::unique_ptr<Stmt>> statements;
};

class IfStmt : public Stmt {
 public:
  IfStmt(std::unique_ptr<Expr> condition,
         std::unique_ptr<BlockStmt> thenBlock,
         std::unique_ptr<BlockStmt> elseBlock)
      : condition(std::move(condition)),
        thenBlock(std::move(thenBlock)),
        elseBlock(std::move(elseBlock)) {}
  void codegen(CodeGenContext& context) override;
 private:
  std::unique_ptr<Expr> condition;
  std::unique_ptr<BlockStmt> thenBlock;
  std::unique_ptr<BlockStmt> elseBlock;  // May be null
};

class WhileStmt : public Stmt {
 public:
  WhileStmt(std::unique_ptr<Expr> condition, std::unique_ptr<BlockStmt> body)
      : condition(std::move(condition)), body(std::move(body)) {}
  void codegen(CodeGenContext& context) override;
 private:
  std::unique_ptr<Expr> condition;
  std::unique_ptr<BlockStmt> body;
};

struct Prototype {
  std::string name;
  std::vector<std::string> argNames;
};

class FunctionAST {
 public:
  FunctionAST(Prototype proto, std::unique_ptr<BlockStmt> body)
      : proto(std::move(proto)), body(std::move(body)) {}
  void codegen(CodeGenContext& context);
  const std::string& getName() const { return proto.name; }
 private:
  Prototype proto;
  std::unique_ptr<BlockStmt> body;
};

}  // namespace mini
