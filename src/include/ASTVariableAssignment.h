//
// Created by zack on 7/9/21.
//

#include "parser.h"
#ifndef STELLAR_ASTVARIABLEASSIGNMENT_H
#define STELLAR_ASTVARIABLEASSIGNMENT_H

using namespace std;

class ASTVariableAssignment : public ASTNode {
    string name;
    ASTNode* value;
public:
    ASTVariableAssignment(string name, ASTNode* value) : name(move(name)), value(value) {};
    string toString() override {
        return "[VAR_ASSIGN: " + name + " " + value->toString() + "]";
    }
    llvm::Value *codegen(llvm::IRBuilder<>* builder,
                         llvm::LLVMContext* context,
                         llvm::BasicBlock* entryBlock,
                         std::map<std::string, llvm::Value*>* namedValues,
                         llvm::Module* module,
                         map<string, string>* objectTypes,
                         map<string, ClassData>* classes) override;
};



#endif //STELLAR_ASTVARIABLEASSIGNMENT_H
