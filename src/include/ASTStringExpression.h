//
// Created by zack on 7/14/21.
//

#include "parser.h"
#include <string>
#include <utility>
#ifndef STELLAR_ASTSTRINGEXPRESSION_H
#define STELLAR_ASTSTRINGEXPRESSION_H

using namespace std;

class ASTStringExpression : public ASTNode {
    string val;
public:
    explicit ASTStringExpression(string val) : val(move(val)) {}
    string toString() override {
        return "[STRING: " + val + "]";
    }
    llvm::Value* codegen(llvm::IRBuilder<>* builder,
                         llvm::LLVMContext* context,
                         llvm::BasicBlock* entryBlock,
                         map<string, llvm::Value*>* namedValues,
                         llvm::Module* module) override;
};


#endif //STELLAR_ASTSTRINGEXPRESSION_H
