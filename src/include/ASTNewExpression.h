//
// Created by zack on 7/18/21.
//

#include "parser.h"
#ifndef STELLAR_ASTNEWEXPRESSION_H
#define STELLAR_ASTNEWEXPRESSION_H

class ASTNewExpression : public ASTNode {
public:
    VariableType classType;
    vector<ASTNode*> args;
    ASTNewExpression(VariableType classType, vector<ASTNode*> args) : classType(move(classType)), args(move(args)) {}
    string toString() override {
        return "[CLASS_INST: type: " + convertVariableTypeToString(classType) + " num args: " + to_string(args.size()) + "]";
    }
    llvm::Value* codegen(CodegenData data) override;
    ASTNodeType getType() override {
        return AST_NEW_EXPRESSION;
    }
};

#endif //STELLAR_ASTNEWEXPRESSION_H
