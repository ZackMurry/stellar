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
    OPERATOR_NE
};

class ASTBinaryExpression : public ASTNode {
    ExpressionOperator op;
    ASTNode *lhs, *rhs;
public:
    ASTBinaryExpression(ExpressionOperator op, ASTNode* lhs, ASTNode* rhs) : op(op), lhs(lhs), rhs(rhs) {}
    string toString() override {
        return "[BIN_EXPRESSION: " + to_string(op) + " " + lhs->toString() + " " + rhs->toString() + "]";
    }
    llvm::Value* codegen(llvm::IRBuilder<>* builder,
                         llvm::LLVMContext* context,
                         llvm::BasicBlock* entryBlock,
                         map<string, llvm::Value*>* namedValues,
                         llvm::Module* module,
                         map<string, string>* objectTypes,
                         map<string, ClassData>* classes) override;
};

#endif //STELLAR_ASTBINARYEXPRESSION_H
