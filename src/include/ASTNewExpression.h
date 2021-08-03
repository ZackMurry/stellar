//
// Created by zack on 7/18/21.
//

#include "parser.h"
#ifndef STELLAR_ASTNEWEXPRESSION_H
#define STELLAR_ASTNEWEXPRESSION_H

class ASTNewExpression : public ASTNode {
public:
    string className;
    vector<ASTNode*> args;
    vector<VariableType> genericTypes;
    ASTNewExpression(string className, vector<ASTNode*> args, vector<VariableType> genericTypes) : className(move(className)), args(move(args)), genericTypes(move(genericTypes)) {}
    string toString() override {
        return "[CLASS_INST: type: " + className + " num args: " + to_string(args.size()) + "]";
    }
    llvm::Value* codegen(CodegenData data) override;
    ASTNodeType getType() override {
        return AST_NEW_EXPRESSION;
    }
};

#endif //STELLAR_ASTNEWEXPRESSION_H
