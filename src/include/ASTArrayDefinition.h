//
// Created by zack on 7/9/21.
//

#include "parser.h"
#ifndef STELLAR_ASTARRAYDEFINITION_H
#define STELLAR_ASTARRAYDEFINITION_H

using namespace std;

class ASTArrayDefinition : public ASTNode {
    string name;
    VariableType elementType;
    ASTNode* length;
public:
    ASTArrayDefinition(string name, VariableType elementType, ASTNode* length) : name(move(name)), elementType(elementType), length(length) {}
    string toString() override {
        return "[ARR_DEF: " + name + " " + to_string(elementType) + " size: " + length->toString() + "]";
    }
    llvm::Value* codegen(llvm::IRBuilder<>* builder,
                         llvm::LLVMContext* context,
                         llvm::BasicBlock* entryBlock,
                         map<string, llvm::Value*> namedValues,
                         llvm::Module* module) override;
};


#endif //STELLAR_ASTARRAYDEFINITION_H
