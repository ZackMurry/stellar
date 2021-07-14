//
// Created by zack on 7/9/21.
//

#include "parser.h"
#ifndef STELLAR_ASTRETURN_H
#define STELLAR_ASTRETURN_H

using namespace std;

class ASTReturn : public ASTNode {
    ASTNode* exp;
public:
    explicit ASTReturn(ASTNode* exp) : exp(exp) {}
    string toString() override {
        if (!exp) {
            return "[RETURN: void]";
        }
        return "[RETURN: " + exp->toString() + "]";
    }
    llvm::Value* codegen(llvm::IRBuilder<>* builder,
                         llvm::LLVMContext* context,
                         llvm::BasicBlock* entryBlock,
                         map<string, llvm::Value*>* namedValues,
                         llvm::Module* module) override;
};


#endif //STELLAR_ASTRETURN_H
