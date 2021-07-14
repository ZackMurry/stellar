//
// Created by zack on 7/9/21.
//

#include "parser.h"
#ifndef STELLAR_ASTFUNCTIONINVOCATION_H
#define STELLAR_ASTFUNCTIONINVOCATION_H

using namespace std;

class ASTFunctionInvocation : public ASTNode {
    string name;
    vector<ASTNode*> args;
public:
    ASTFunctionInvocation(string name, vector<ASTNode*> args) : name(move(name)), args(move(args)) {}
    string toString() override {
        string s = "[FUN_INV: " + name + " args: [";
        for (auto const& arg : args) {
            s += arg->toString();
        }
        s += "]]";
        return s;
    }
    llvm::Value* codegen(llvm::IRBuilder<>* builder,
                         llvm::LLVMContext* context,
                         llvm::BasicBlock* entryBlock,
                         map<string, llvm::Value*>* namedValues,
                         llvm::Module* module) override;
};



#endif //STELLAR_ASTFUNCTIONINVOCATION_H
