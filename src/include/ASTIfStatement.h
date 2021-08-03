//
// Created by zack on 7/9/21.
//

#include "parser.h"
#include "ASTBinaryExpression.h"
#ifndef STELLAR_ASTIFSTATEMENT_H
#define STELLAR_ASTIFSTATEMENT_H


class ASTIfStatement : public ASTNode {
public:
    ASTNode* condition;
    vector<ASTNode*> ifBody;
    vector<ASTNode*> elseBody;
    ASTIfStatement(ASTNode* condition, vector<ASTNode*> ifBody, vector<ASTNode*> elseBody) : condition(condition), ifBody(move(ifBody)), elseBody(move(elseBody)) {}
    string toString() override {
        string s = "[IF_STMT: condition: " + condition->toString() + " ifBody: [";
        for (auto const& stmt : ifBody) {
            s += stmt->toString();
        }
        s += "] elseBody: [";
        for (auto const& stmt : elseBody) {
            s += stmt->toString();
        }
        s += "]]";
        return s;
    }
    llvm::Value *codegen(CodegenData data) override;
    ASTNodeType getType() override {
        return AST_IF_STATEMENT;
    }
};



#endif //STELLAR_ASTIFSTATEMENT_H
