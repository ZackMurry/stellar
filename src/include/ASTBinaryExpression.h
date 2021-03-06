//
// Created by zack on 7/9/21.
//

#include "parser.h"
#ifndef STELLAR_ASTBINARYEXPRESSION_H
#define STELLAR_ASTBINARYEXPRESSION_H

using namespace std;

enum ExpressionOperator {
    OPERATOR_PLUS,
    OPERATOR_MINUS,
    OPERATOR_TIMES,
    OPERATOR_DIVIDE,
    OPERATOR_LT,
    OPERATOR_GT,
    OPERATOR_LE,
    OPERATOR_GE,
    OPERATOR_EQ,
    OPERATOR_NE,
    OPERATOR_MODULO,
    OPERATOR_AND,
    OPERATOR_OR
};

class ASTBinaryExpression : public ASTNode {
public:
    ExpressionOperator op;
    ASTNode *lhs, *rhs;
    ASTBinaryExpression(ExpressionOperator op, ASTNode* lhs, ASTNode* rhs) : op(op), lhs(lhs), rhs(rhs) {}
    string toString() override {
        return "[BIN_EXP: " + to_string(op) + " " + lhs->toString() + " " + rhs->toString() + "]";
    }
    llvm::Value* codegen(CodegenData data) override;
    ASTNodeType getType() override {
        return AST_BINARY_EXPRESSION;
    }
};

#endif //STELLAR_ASTBINARYEXPRESSION_H
