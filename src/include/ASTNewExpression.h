//
// Created by zack on 7/18/21.
//

#include "parser.h"
#ifndef STELLAR_ASTNEWEXPRESSION_H
#define STELLAR_ASTNEWEXPRESSION_H

class ASTNewExpression : public ASTNode {
    string className;
public:
    explicit ASTNewExpression(string className) : className(move(className)) {}
    string toString() override {
        return "[CLASS_INST: type: " + className + "]";
    }
    llvm::Value* codegen(llvm::IRBuilder<>* builder,
                         llvm::LLVMContext* context,
                         llvm::BasicBlock* entryBlock,
                         map<string, llvm::Value*>* namedValues,
                         llvm::Module* module,
                         map<string, string>* objectTypes,
                         map<string, ClassData>* classes) override;
};

#endif //STELLAR_ASTNEWEXPRESSION_H
