//
// Created by zack on 7/25/21.
//

#include "parser.h"
#include "ASTBinaryExpression.h"
#ifndef STELLAR_ASTFOREXPRESSION_H
#define STELLAR_ASTFOREXPRESSION_H

class ASTForExpression : public ASTNode {
public:
    ASTNode* initializer;
    ASTNode* condition;
    ASTNode* action;
    vector<ASTNode*> body;
    ASTForExpression(ASTNode* initializer, ASTNode* condition, ASTNode* action, vector<ASTNode*> body) : initializer(initializer), condition(condition), action(action), body(move(body)) {}
    string toString() override {
        string s = "[FOR: init: " + (initializer ? initializer->toString() : "none") + " condition: " + (condition ? condition->toString() : "none") + " action: " + (action ? action->toString() : "none") + " body: [";
        for (const auto& b : body) {
            s += b->toString();
        }
        return s + "]]";
    }
    llvm::Value *codegen(CodegenData data) override;
    ASTNodeType getType() override {
        return AST_FOR_EXPRESSION;
    }
};

#endif //STELLAR_ASTFOREXPRESSION_H
