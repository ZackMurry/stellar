//
// Created by zack on 7/9/21.
//

#include "parser.h"
#ifndef STELLAR_ASTARRAYINDEXASSIGNMENT_H
#define STELLAR_ASTARRAYINDEXASSIGNMENT_H

using namespace std;

class ASTArrayIndexAssignment : public ASTNode {
    string name;
    ASTNode* index;
    ASTNode* value;
public:
    ASTArrayIndexAssignment(string name, ASTNode* index, ASTNode* value) : name(move(name)), index(index), value(value) {}
    string toString() override {
        return "[ARR_ASSIGN: " + name + " at " + index->toString() + " to " + value->toString() + "]";
    }
    llvm::Value* codegen(llvm::IRBuilder<>* builder,
                         llvm::LLVMContext* context,
                         llvm::BasicBlock* entryBlock,
                         map<string, llvm::Value*>* namedValues,
                         llvm::Module* module) override;
};

#endif //STELLAR_ASTARRAYINDEXASSIGNMENT_H
