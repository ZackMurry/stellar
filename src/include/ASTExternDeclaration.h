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
    vector<string> argTypes;
    VariableType returnType;
public:
    ASTExternDeclaration(string name, vector<string> argTypes, VariableType returnType) : name(move(name)), argTypes(move(argTypes)), returnType(returnType) {}
    string toString() override {
        string s = "[EXTERN: " + to_string(returnType) + " " + name + " args: [";
        for (const auto& argType : argTypes) {
            s += argType;
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
};

#endif //STELLAR_ASTEXTERNDECLARATION_H
