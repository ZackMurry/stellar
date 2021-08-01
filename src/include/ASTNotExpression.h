//
// Created by zack on 8/1/21.
//

#include "parser.h"
#ifndef STELLAR_ASTNOTEXPRESSION_H
#define STELLAR_ASTNOTEXPRESSION_H

class ASTNotExpression : public ASTNode {
    ASTNode* value;
public:
    explicit ASTNotExpression(ASTNode* value) : value(value) {}
    string toString() override {
        return "[NOT: " + value->toString() + "]";
    }
    llvm::Value* codegen(llvm::IRBuilder<>* builder,
                         llvm::LLVMContext* context,
                         llvm::BasicBlock* entryBlock,
                         map<string, llvm::Value*>* namedValues,
                         llvm::Module* module,
                         map<string, string>* objectTypes,
                         map<string, ClassData>* classes) override;
};


#endif //STELLAR_ASTNOTEXPRESSION_H
