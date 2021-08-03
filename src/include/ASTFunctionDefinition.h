//
// Created by zack on 7/9/21.
//

#include <utility>

#include "parser.h"
#include "ASTVariableDefinition.h"

#ifndef STELLAR_ASTFUNCTIONDEFINITION_H
#define STELLAR_ASTFUNCTIONDEFINITION_H

using namespace std;

class ASTFunctionDefinition : public ASTNode {
public:
    string name;
    vector<ASTVariableDefinition*> args;
    vector<ASTNode*> body;
    VariableType returnType;
    ASTFunctionDefinition(string name, vector<ASTVariableDefinition*> args, vector<ASTNode*> body, VariableType returnType) : name(move(name)), args(move(args)), body(move(body)), returnType(move(returnType)) {}
    string toString() override {
        string s = "[FUN_DEF: " + name + " args: [";
        for (const auto& arg : args) {
            s += arg->toString();
        }
        s += "] body: [";
        for (const auto& line : body) {
            s += line->toString();
        }
        s += "] returnType: " + convertVariableTypeToString(returnType) + "]";
        return s;
    }
    llvm::Value *codegen(CodegenData data) override;
    ASTNodeType getType() override {
        return AST_FUNCTION_DEFINITION;
    }
};


#endif //STELLAR_ASTFUNCTIONDEFINITION_H
