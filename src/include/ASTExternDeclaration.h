//
// Created by zack on 7/9/21.
//

#include "parser.h"
#include "ASTVariableDefinition.h"
#ifndef STELLAR_ASTEXTERNDECLARATION_H
#define STELLAR_ASTEXTERNDECLARATION_H

using namespace std;

class ASTExternDeclaration : public ASTNode {
public:
    string name;
    vector<string> argTypes;
    PrimitiveVariableType returnType;
    bool isVarArgs;
    ASTExternDeclaration(string name, vector<string> argTypes, PrimitiveVariableType returnType, bool isVarArgs) : name(move(name)), argTypes(move(argTypes)), returnType(returnType), isVarArgs(isVarArgs) {}
    string toString() override {
        string s = "[EXTERN: " + to_string(returnType) + " " + name + " args: [";
        for (const auto& argType : argTypes) {
            s += argType;
        }
        if (isVarArgs) {
            s += "] isVarArgs: [true";
        }
        s += "]]";
        return s;
    }
    llvm::Value *codegen(CodegenData data) override;
    ASTNodeType getType() override {
        return AST_EXTERN_DECLARATION;
    }
};

#endif //STELLAR_ASTEXTERNDECLARATION_H
