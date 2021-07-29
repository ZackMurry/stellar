//
// Created by zack on 7/29/21.
//

#include "parser.h"
#ifndef STELLAR_ASTBOOLEANEXPRESSION_H
#define STELLAR_ASTBOOLEANEXPRESSION_H

class ASTBooleanExpression : public ASTNode {
    bool value;
public:
    explicit ASTBooleanExpression(bool value) : value(value) {}
    string toString() override {
        string s = "[BOOL: ";
        if (value) {
            s += "true";
        } else {
            s += "false";
        }
        return s + "]";
    }
    llvm::Value *codegen(llvm::IRBuilder<>* builder,
                         llvm::LLVMContext* context,
                         llvm::BasicBlock* entryBlock,
                         std::map<std::string, llvm::Value*>* namedValues,
                         llvm::Module* module,
                         map<string, string>* objectTypes,
                         map<string, ClassData>* classes) override;
};


#endif //STELLAR_ASTBOOLEANEXPRESSION_H
