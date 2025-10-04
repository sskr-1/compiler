#include "codegen.h"
#include "ast.h"
#include <iostream>

// CodeGenerator implementation
CodeGenerator::CodeGenerator(const std::string& module_name) : builder(context) {
    module = std::make_unique<llvm::Module>(module_name, context);
}

llvm::Type* CodeGenerator::getLLVMType(Type* type) {
    return getLLVMType(type->type_name);
}

llvm::Type* CodeGenerator::getLLVMType(const std::string& type_name) {
    if (type_name == "int") {
        return llvm::Type::getInt32Ty(context);
    } else if (type_name == "float") {
        return llvm::Type::getFloatTy(context);
    } else if (type_name == "double") {
        return llvm::Type::getDoubleTy(context);
    } else if (type_name == "char") {
        return llvm::Type::getInt8Ty(context);
    } else if (type_name == "void") {
        return llvm::Type::getVoidTy(context);
    } else {
        std::cerr << "Unknown type: " << type_name << std::endl;
        return nullptr;
    }
}

void CodeGenerator::setNamedValue(const std::string& name, llvm::Value* value) {
    named_values[name] = value;
}

llvm::Value* CodeGenerator::getNamedValue(const std::string& name) {
    auto it = named_values.find(name);
    if (it != named_values.end()) {
        return it->second;
    }
    return nullptr;
}

void CodeGenerator::setFunction(const std::string& name, llvm::Function* func) {
    function_decls[name] = func;
}

llvm::Function* CodeGenerator::getFunction(const std::string& name) {
    auto it = function_decls.find(name);
    if (it != function_decls.end()) {
        return it->second;
    }
    return nullptr;
}

llvm::Value* CodeGenerator::generateCode(Expression* expr) {
    return expr->codegen(*this);
}

llvm::Value* CodeGenerator::generateCode(Statement* stmt) {
    return stmt->codegen(*this);
}

llvm::Value* CodeGenerator::generateCode(Declaration* decl) {
    return decl->codegen(*this);
}

llvm::Value* CodeGenerator::generateCode(Program* program) {
    return program->codegen(*this);
}

void CodeGenerator::printIR(const std::string& filename) {
    std::error_code error_code;
    llvm::raw_fd_ostream out(filename, error_code);

    if (error_code) {
        std::cerr << "Could not open file: " << filename << std::endl;
        return;
    }

    module->print(out, nullptr);
    out.close();
}

void CodeGenerator::printIRToStdout() {
    module->print(llvm::outs(), nullptr);
}

llvm::Value* CodeGenerator::logError(const std::string& str) {
    std::cerr << "Error: " << str << std::endl;
    return nullptr;
}

llvm::Function* CodeGenerator::logErrorF(const std::string& str) {
    logError(str);
    return nullptr;
}