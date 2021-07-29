//
// Created by zack on 7/29/21.
//

#include <utility>

#include "parser.h"
#ifndef STELLAR_ASTNULLCHECKEXPRESSION_H
#define STELLAR_ASTNULLCHECKEXPRESSION_H

class ASTNullCheckExpression : public ASTNode {
    ASTNode* value;
public:
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
};


#endif //STELLAR_ASTNULLCHECKEXPRESSION_H
