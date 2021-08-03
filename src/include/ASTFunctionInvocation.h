//
// Created by zack on 7/9/21.
//

#include "parser.h"
#ifndef STELLAR_ASTFUNCTIONINVOCATION_H
#define STELLAR_ASTFUNCTIONINVOCATION_H

using namespace std;

class ASTFunctionInvocation : public ASTNode {
public:
    string name;
    vector<ASTNode*> args;
    ASTFunctionInvocation(string name, vector<ASTNode*> args) : name(move(name)), args(move(args)) {}
    string toString() override {
        string s = "[FUN_INV: " + name + " args: [";
        for (auto const& arg : args) {
            s += arg->toString();
        }
        s += "]]";
        return s;
    }
    llvm::Value *codegen(llvm::IRBuilder<>* builder,
                         llvm::LLVMContext* context,
                         llvm::BasicBlock* entryBlock,
                         std::map<std::string, llvm::Value*>* namedValues,
                         llvm::Module* module,
                         map<string, string>* objectTypes,
                         map<string, ClassData>* classes) override;
    ASTNodeType getType() override {
        return AST_FUNCTION_INVOCATION;
    }
};



#endif //STELLAR_ASTFUNCTIONINVOCATION_H
