//
// Created by zack on 7/1/21.
//

#pragma once
#include <vector>
#include <llvm/IR/IRBuilder.h>
#include "LexerToken.h"
#include "llvm/IR/Constant.h"
#include <iostream>
#include <map>

#ifndef STELLAR_PARSER_H
#define STELLAR_PARSER_H

using namespace std;

enum VariableType {
    VARIABLE_TYPE_I8,
    VARIABLE_TYPE_I16,
    VARIABLE_TYPE_I32,
    VARIABLE_TYPE_I64,
    VARIABLE_TYPE_F,
    VARIABLE_TYPE_D,
    VARIABLE_TYPE_B,
    VARIABLE_TYPE_V,
    VARIABLE_TYPE_S
};

struct ClassData {
    llvm::Type* type;
    map<string, llvm::Type*> fields;
    map<string, llvm::Function*> methods;
};

llvm::Type* getLLVMTypeByVariableType(VariableType type, llvm::LLVMContext* context);

int getVariableTypeFromString(const string& type);

class ASTNode {
public:
    ~ASTNode() = default;
    virtual std::string toString() {
        return "ROOT_NODE";
    }
    virtual llvm::Value* codegen(llvm::IRBuilder<>* builder,
                         llvm::LLVMContext* context,
                         llvm::BasicBlock* entryBlock,
                         std::map<std::string, llvm::Value*>* namedValues,
                         llvm::Module* module,
                         map<string, string>* objectTypes,
                         map<string, ClassData>* classes) = 0;
};

std::vector<ASTNode*> parse(std::vector<Token> tokens);

#endif //STELLAR_PARSER_H
