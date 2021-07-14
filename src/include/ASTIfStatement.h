//
// Created by zack on 7/9/21.
//

#include "parser.h"
#include "ASTBinaryExpression.h"
#ifndef STELLAR_ASTIFSTATEMENT_H
#define STELLAR_ASTIFSTATEMENT_H

using namespace std;

// todo: logical and
class ASTIfStatement : public ASTNode {
    ASTBinaryExpression* condition;
    vector<ASTNode*> ifBody;
    vector<ASTNode*> elseBody;
public:
    ASTIfStatement(ASTBinaryExpression* condition, vector<ASTNode*> ifBody, vector<ASTNode*> elseBody) : condition(condition), ifBody(move(ifBody)), elseBody(move(elseBody)) {}
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
    llvm::Value* codegen(llvm::IRBuilder<>* builder,
                         llvm::LLVMContext* context,
                         llvm::BasicBlock* entryBlock,
                         map<string, llvm::Value*>* namedValues,
                         llvm::Module* module) override;
};



#endif //STELLAR_ASTIFSTATEMENT_H
