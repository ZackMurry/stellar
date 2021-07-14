//
// Created by zack on 7/9/21.
//

#include "parser.h"
#ifndef STELLAR_ASTNUMBEREXPRESSION_H
#define STELLAR_ASTNUMBEREXPRESSION_H

using namespace std;

class ASTNumberExpression : public ASTNode {
    string val;
    VariableType type;
public:
    ASTNumberExpression(string val, VariableType type) : val(move(val)), type(type) {}
    string toString() override {
        return "[NUM_EXP: " + val + " type: " + to_string(type) + "]";
    }
    llvm::Value *codegen(llvm::IRBuilder<>* builder,
                         llvm::LLVMContext* context,
                         llvm::BasicBlock* entryBlock,
                         map<string, llvm::Value*>* namedValues,
                         llvm::Module* module) override;
};


#endif //STELLAR_ASTNUMBEREXPRESSION_H
