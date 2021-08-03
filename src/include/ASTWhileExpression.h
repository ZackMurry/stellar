//
// Created by zack on 7/25/21.
//

#include "parser.h"
#ifndef STELLAR_ASTWHILEEXPRESSION_H
#define STELLAR_ASTWHILEEXPRESSION_H

class ASTWhileExpression : public ASTNode {
public:
    ASTNode* condition;
    vector<ASTNode*> body;
    ASTWhileExpression(ASTNode* condition, vector<ASTNode*> body) : condition(condition), body(move(body)) {}
    string toString() override {
        string s = "[WHILE: condition: " + condition->toString() + " body: [";
        for (auto const& stmt : body) {
            s += stmt->toString();
        }
        return s + "]]";
    }
    llvm::Value *codegen(CodegenData data) override;
    ASTNodeType getType() override {
        return AST_WHILE_EXPRESSION;
    }
};

#endif //STELLAR_ASTWHILEEXPRESSION_H
