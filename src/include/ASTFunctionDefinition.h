//
// Created by zack on 7/9/21.
//

#include "parser.h"
#include "ASTVariableDefinition.h"

#ifndef STELLAR_ASTFUNCTIONDEFINITION_H
#define STELLAR_ASTFUNCTIONDEFINITION_H

using namespace std;

class ASTFunctionDefinition : public ASTNode {
    string name;
    vector<ASTVariableDefinition*> args;
    vector<ASTNode*> body;
    VariableType returnType;
public:
    ASTFunctionDefinition(string name, vector<ASTVariableDefinition*> args, vector<ASTNode*> body, VariableType returnType) : name(move(name)), args(move(args)), body(move(body)), returnType(returnType) {}
    string toString() override {
        string s = "[FUN_DEF: " + name + " args: [";
        for (const auto& arg : args) {
            s += arg->toString();
        }
        s += "] body: [";
        for (const auto& line : body) {
            s += line->toString();
        }
        s += "] ]";
        return s;
    }
    llvm::Value* codegen(llvm::IRBuilder<>* builder,
                         llvm::LLVMContext* context,
                         llvm::BasicBlock* entryBlock,
                         map<string, llvm::Value*> namedValues,
                         llvm::Module* module) override;
};


#endif //STELLAR_ASTFUNCTIONDEFINITION_H
