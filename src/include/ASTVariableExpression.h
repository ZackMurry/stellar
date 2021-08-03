//
// Created by zack on 7/9/21.
//

#include "parser.h"
#ifndef STELLAR_ASTVARIABLEEXPRESSION_H
#define STELLAR_ASTVARIABLEEXPRESSION_H

using namespace std;

class ASTVariableExpression : public ASTNode {
public:
    string name;
    explicit ASTVariableExpression(string name) : name(move(name)) {}
    string toString() override {
        return "[VARIABLE: " + name + "]";
    }
    llvm::Value *codegen(llvm::IRBuilder<>* builder,
                         llvm::LLVMContext* context,
                         llvm::BasicBlock* entryBlock,
                         map<string, llvm::Value*>* namedValues,
                         llvm::Module* module,
                         map<string, string>* objectTypes,
                         map<string, ClassData>* classes) override;
    ASTNodeType getType() override {
        return AST_VARIABLE_EXPRESSION;
    }
};

#endif //STELLAR_ASTVARIABLEEXPRESSION_H
