//
// Created by zack on 7/15/21.
//

#include "parser.h"
#include <string>
#ifndef STELLAR_ASTCLASSINSTANTIATION_H
#define STELLAR_ASTCLASSINSTANTIATION_H

using namespace std;

class ASTClassInstantiation : public ASTNode {
    llvm::Type* type;
    string name;
public:
    ASTClassInstantiation(llvm::Type* type, string name) : type(type), name(move(name)) {}
    string toString() override {
        return "[CLASS_INST: type: " + type->getStructName().str() + " name: " + name + "]";
    }
    llvm::Value* codegen(llvm::IRBuilder<>* builder,
                         llvm::LLVMContext* context,
                         llvm::BasicBlock* entryBlock,
                         map<string, llvm::Value*>* namedValues,
                         llvm::Module* module) override;
};


#endif //STELLAR_ASTCLASSINSTANTIATION_H
