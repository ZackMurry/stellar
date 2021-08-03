//
// Created by zack on 7/29/21.
//

#include <utility>

#include "parser.h"
#ifndef STELLAR_ASTNULLCHECKEXPRESSION_H
#define STELLAR_ASTNULLCHECKEXPRESSION_H

class ASTNullCheckExpression : public ASTNode {
public:
    ASTNode* value;
    explicit ASTNullCheckExpression(ASTNode* value) : value(value) {}
    string toString() override {
        return "[IS_NULL: " + value->toString() + "]";
    }
    llvm::Value *codegen(llvm::IRBuilder<>* builder,
                         llvm::LLVMContext* context,
                         llvm::BasicBlock* entryBlock,
                         std::map<std::string, llvm::Value*>* namedValues,
                         llvm::Module* module,
                         map<string, string>* objectTypes,
                         map<string, ClassData>* classes) override;
    ASTNodeType getType() override {
        return AST_NULL_CHECK_EXPRESSION;
    }
};


#endif //STELLAR_ASTNULLCHECKEXPRESSION_H
