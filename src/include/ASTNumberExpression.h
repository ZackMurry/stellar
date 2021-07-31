//
// Created by zack on 7/9/21.
//

#include "parser.h"
#ifndef STELLAR_ASTNUMBEREXPRESSION_H
#define STELLAR_ASTNUMBEREXPRESSION_H

using namespace std;

class ASTNumberExpression : public ASTNode {
    string val;
    VariableType type;
public:
    ASTNumberExpression(string val, VariableType type) : val(move(val)), type(type) {}
    string toString() override {
        return "[NUMBER: " + val + " type: " + to_string(type) + "]";
    }
    llvm::Value *codegen(llvm::IRBuilder<>* builder,
                         llvm::LLVMContext* context,
                         llvm::BasicBlock* entryBlock,
                         std::map<std::string, llvm::Value*>* namedValues,
                         llvm::Module* module,
                         map<string, string>* objectTypes,
                         map<string, ClassData>* classes) override;
};


#endif //STELLAR_ASTNUMBEREXPRESSION_H
