//
// Created by zack on 8/1/21.
//

#include "parser.h"
#ifndef STELLAR_ASTLOGICALEXPRESSION_H
#define STELLAR_ASTLOGICALEXPRESSION_H

enum LogicalConjunction {
    CONJUNCTION_AND,
    CONJUNCTION_OR
};

class ASTLogicalExpression : public ASTNode {
    LogicalConjunction conjunction;
    ASTNode* lhs;
    ASTNode* rhs;
public:
    ASTLogicalExpression(LogicalConjunction conjunction, ASTNode* lhs, ASTNode* rhs) : conjunction(conjunction), lhs(lhs), rhs(rhs) {}
    string toString() override {
        return "[LOGIC_EXP: " + to_string(conjunction) + " lhs: " + lhs->toString() + " " + rhs->toString() + "]";
    }
    llvm::Value *codegen(llvm::IRBuilder<>* builder,
                         llvm::LLVMContext* context,
                         llvm::BasicBlock* entryBlock,
                         std::map<std::string, llvm::Value*>* namedValues,
                         llvm::Module* module,
                         map<string, string>* objectTypes,
                         map<string, ClassData>* classes) override;
};


#endif //STELLAR_ASTLOGICALEXPRESSION_H
