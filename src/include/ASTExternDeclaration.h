//
// Created by zack on 7/9/21.
//

#include "parser.h"
#include "ASTVariableDefinition.h"
#ifndef STELLAR_ASTEXTERNDECLARATION_H
#define STELLAR_ASTEXTERNDECLARATION_H

using namespace std;

class ASTExternDeclaration : public ASTNode {
    string name;
    vector<ASTVariableDefinition*> args;
    VariableType returnType;
public:
    ASTExternDeclaration(string name, vector<ASTVariableDefinition*> args, VariableType returnType) : name(move(name)), args(move(args)), returnType(returnType) {}
    string toString() override {
        string s = "[EXTERN: " + to_string(returnType) + " " + name + " args: [";
        for (const auto& arg : args) {
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

#endif //STELLAR_ASTEXTERNDECLARATION_H
