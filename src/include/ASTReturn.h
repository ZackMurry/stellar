//
// Created by zack on 7/9/21.
//

#include "parser.h"
#ifndef STELLAR_ASTRETURN_H
#define STELLAR_ASTRETURN_H

using namespace std;

class ASTReturn : public ASTNode {
public:
    ASTNode* exp;
    explicit ASTReturn(ASTNode* exp) : exp(exp) {}
    string toString() override {
        if (!exp) {
            return "[RETURN: void]";
        }
        return "[RETURN: " + exp->toString() + "]";
    }
    llvm::Value *codegen(CodegenData data) override;
    ASTNodeType getType() override {
        return AST_RETURN;
    }
};


#endif //STELLAR_ASTRETURN_H
