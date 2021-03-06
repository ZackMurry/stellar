//
// Created by zack on 7/9/21.
//

#include "parser.h"
#ifndef STELLAR_ASTNUMBEREXPRESSION_H
#define STELLAR_ASTNUMBEREXPRESSION_H

using namespace std;

class ASTNumberExpression : public ASTNode {
public:
    string val;
    PrimitiveVariableType type;
    ASTNumberExpression(string val, PrimitiveVariableType type) : val(move(val)), type(type) {}
    string toString() override {
        return "[NUMBER: " + val + " type: " + to_string(type) + "]";
    }
    llvm::Value *codegen(CodegenData data) override;
    ASTNodeType getType() override {
        return AST_NUMBER_EXPRESSION;
    }
};


#endif //STELLAR_ASTNUMBEREXPRESSION_H
