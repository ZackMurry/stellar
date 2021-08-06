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

enum ASTNodeType {
    AST_NODE,
    AST_ARRAY_ACCESS,
    AST_ARRAY_DEFINITION,
    AST_ARRAY_INDEX_ASSIGNMENT,
    AST_BINARY_EXPRESSION,
    AST_BOOLEAN_EXPRESSION,
    AST_CLASS_DEFINITION,
    AST_CLASS_FIELD_ACCESS,
    AST_CLASS_FIELD_STORE,
    AST_CLASS_INSTANTIATION,
    AST_EXTERN_DECLARATION,
    AST_FOR_EXPRESSION,
    AST_FUNCTION_DEFINITION,
    AST_FUNCTION_INVOCATION,
    AST_IF_STATEMENT,
    AST_METHOD_CALL,
    AST_NEW_EXPRESSION,
    AST_NOT_EXPRESSION,
    AST_NULL_CHECK_EXPRESSION,
    AST_NUMBER_EXPRESSION,
    AST_RETURN,
    AST_STRING_EXPRESSION,
    AST_VARIABLE_ASSIGNMENT,
    AST_VARIABLE_DECLARATION,
    AST_VARIABLE_DEFINITION,
    AST_VARIABLE_EXPRESSION,
    AST_VARIABLE_MUTATION,
    AST_WHILE_EXPRESSION
};

enum PrimitiveVariableType {
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

struct VariableType {
    string type;
    vector<VariableType> genericTypes;
};

string convertVariableTypeToString(VariableType v);

struct ClassFieldDefinition {
    string name;
    VariableType type;
};

struct ClassFieldType {
    string name;
    string classType;
    llvm::Type* type;
};

struct ClassData {
    llvm::Type* type;
    vector<ClassFieldType> fields;
    vector<string> methodOrder;
    map<string, llvm::Function*> methods;
    string parent;
    llvm::Type* vtableType;
    llvm::Value* vtableGlobal;
};

llvm::Type* getLLVMTypeByPrimitiveVariableType(PrimitiveVariableType type, llvm::LLVMContext* context);

int getPrimitiveVariableTypeFromString(const string& type);

struct CodegenData {
    llvm::IRBuilder<>* builder;
    llvm::LLVMContext* context;
    llvm::BasicBlock* entryBlock;
    map<string, llvm::Value*>* namedValues;
    llvm::Module* module;
    map<string, string>* objectTypes;
    map<string, ClassData>* classes;
    map<string, VariableType>* generics;
};

class ASTNode {
public:
    ~ASTNode() = default;
    virtual string toString() {
        return "ROOT_NODE";
    }
    virtual llvm::Value* codegen(CodegenData data) = 0;
    virtual ASTNodeType getType() {
        return AST_NODE;
    }
};

std::vector<ASTNode*> parse(std::vector<Token> tokens);

#endif //STELLAR_PARSER_H
