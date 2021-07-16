//
// Created by zack on 7/9/21.
//

#include "parser.h"
#include <iostream>
#ifndef STELLAR_ASTARRAYACCESS_H
#define STELLAR_ASTARRAYACCESS_H

using namespace std;

llvm::Type* getLLVMPtrTypeByType(llvm::Type* type, llvm::LLVMContext* context);

class ASTArrayAccess : public ASTNode {
    string name;
    ASTNode* index;
public:
    ASTArrayAccess(string name, ASTNode* index) : name(move(name)), index(index) {}
    string toString() override {
        return "[ARR_ACCESS: " + name + " at " + index->toString() + "]";
    }
    llvm::Value* codegen(llvm::IRBuilder<>* builder,
                         llvm::LLVMContext* context,
                         llvm::BasicBlock* entryBlock,
                         map<string, llvm::Value*>* namedValues,
                         llvm::Module* module) override;
};


#endif //STELLAR_ASTARRAYACCESS_H