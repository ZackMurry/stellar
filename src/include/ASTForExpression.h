//
// Created by zack on 7/25/21.
//

#include "parser.h"
#include "ASTBinaryExpression.h"
#ifndef STELLAR_ASTFOREXPRESSION_H
#define STELLAR_ASTFOREXPRESSION_H

// todo: logical and
class ASTForExpression : public ASTNode {
    ASTNode* initializer;
    ASTNode* condition;
    ASTNode* action;
    vector<ASTNode*> body;
public:
    ASTForExpression(ASTNode* initializer, ASTNode* condition, ASTNode* action, vector<ASTNode*> body) : initializer(initializer), condition(condition), action(action), body(move(body)) {}
    string toString() override {
        string s = "[FOR: init: " + (initializer ? initializer->toString() : "none") + " condition: " + (condition ? condition->toString() : "none") + " action: " + (action ? action->toString() : "none") + " body: [";
        for (const auto& b : body) {
            s += b->toString();
        }
        return s + "]]";
    }
    llvm::Value *codegen(llvm::IRBuilder<>* builder,
                         llvm::LLVMContext* context,
                         llvm::BasicBlock* entryBlock,
                         std::map<std::string, llvm::Value*>* namedValues,
                         llvm::Module* module,
                         map<string, string>* objectTypes,
                         map<string, ClassData>* classes) override;
};

#endif //STELLAR_ASTFOREXPRESSION_H
