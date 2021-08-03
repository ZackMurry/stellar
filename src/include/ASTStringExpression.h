//
// Created by zack on 7/14/21.
//

#include "parser.h"
#include <string>
#include <utility>
#ifndef STELLAR_ASTSTRINGEXPRESSION_H
#define STELLAR_ASTSTRINGEXPRESSION_H

using namespace std;

class ASTStringExpression : public ASTNode {
public:
    string val;
    explicit ASTStringExpression(string val) : val(move(val)) {}
    string toString() override {
        return "[STRING: " + val + "]";
    }
    llvm::Value *codegen(CodegenData data) override;
    ASTNodeType getType() override {
        return AST_STRING_EXPRESSION;
    }
};


#endif //STELLAR_ASTSTRINGEXPRESSION_H
