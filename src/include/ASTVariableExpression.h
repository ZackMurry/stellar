//
// Created by zack on 7/9/21.
//

#include "parser.h"
#ifndef STELLAR_ASTVARIABLEEXPRESSION_H
#define STELLAR_ASTVARIABLEEXPRESSION_H

using namespace std;

class ASTVariableExpression : public ASTNode {
    string name;
public:
    explicit ASTVariableExpression(string name) : name(move(name)) {}
    string toString() override {
        return "[VAR_EXP: " + name + "]";
    }
    llvm::Value *codegen(llvm::IRBuilder<>* builder,
                         llvm::LLVMContext* context,
                         llvm::BasicBlock* entryBlock,
                         map<string, llvm::Value*>* namedValues,
                         llvm::Module* module) override;
};

#endif //STELLAR_ASTVARIABLEEXPRESSION_H
