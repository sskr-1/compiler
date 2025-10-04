#include <iostream>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include <llvm/Support/raw_ostream.h>

#include "AST.h"
#include "CodeGen.h"

using namespace mini;

static std::unique_ptr<BlockStmt> buildAddBody() {
  // return a + b;
  auto body = std::make_unique<BlockStmt>();
  auto expr = std::make_unique<BinaryExpr>(BinaryOp::Add,
                                           std::make_unique<VariableExpr>("a"),
                                           std::make_unique<VariableExpr>("b"));
  body->add(std::make_unique<ReturnStmt>(std::move(expr)));
  return body;
}

static std::unique_ptr<BlockStmt> buildMainBody() {
  // { int x = 41; int y = 1; int z = add(x, y); return z; }
  auto body = std::make_unique<BlockStmt>();
  body->add(std::make_unique<VarDeclStmt>("x", std::make_unique<NumberExpr>(41)));
  body->add(std::make_unique<VarDeclStmt>("y", std::make_unique<NumberExpr>(1)));

  std::vector<std::unique_ptr<Expr>> callArgs;
  callArgs.push_back(std::make_unique<VariableExpr>("x"));
  callArgs.push_back(std::make_unique<VariableExpr>("y"));
  auto callAdd = std::make_unique<CallExpr>("add", std::move(callArgs));
  body->add(std::make_unique<VarDeclStmt>("z", std::move(callAdd)));

  body->add(std::make_unique<ReturnStmt>(std::make_unique<VariableExpr>("z")));
  return body;
}

int main() {
  CodeGenContext context("mini-module");

  // Build function: int add(int a, int b)
  Prototype addProto{.name = "add", .argNames = {"a", "b"}};
  FunctionAST addFn(std::move(addProto), buildAddBody());
  addFn.codegen(context);

  // Build function: int main()
  Prototype mainProto{.name = "main", .argNames = {}};
  FunctionAST mainFn(std::move(mainProto), buildMainBody());
  mainFn.codegen(context);

  context.module->print(llvm::outs(), nullptr);
  return 0;
}
