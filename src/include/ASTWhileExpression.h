//
// Created by zack on 7/25/21.
//

#include "parser.h"
#ifndef STELLAR_ASTWHILEEXPRESSION_H
#define STELLAR_ASTWHILEEXPRESSION_H

class ASTWhileExpression : public ASTNode {
    ASTNode* condition;
    vector<ASTNode*> body;
public:
    ASTWhileExpression(ASTNode* condition, vector<ASTNode*> body) : condition(condition), body(move(body)) {}
    string toString() override {
        string s = "[WHILE: condition: " + condition->toString() + " body: [";
        for (auto const& stmt : body) {
            s += stmt->toString();
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

#endif //STELLAR_ASTWHILEEXPRESSION_H
