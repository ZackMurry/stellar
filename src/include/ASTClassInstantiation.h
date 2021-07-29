//
// Created by zack on 7/15/21.
//

#include "parser.h"
#include <string>
#ifndef STELLAR_ASTCLASSINSTANTIATION_H
#define STELLAR_ASTCLASSINSTANTIATION_H

using namespace std;

class ASTClassInstantiation : public ASTNode {
    string className;
    string identifier;
    vector<ASTNode*> args;
public:
    ASTClassInstantiation(string className, string identifier, vector<ASTNode*> args) : className(move(className)), identifier(move(identifier)), args(move(args)) {}
    string toString() override {
        return "[CLASS_INST: type: " + className + " name: " + identifier + " num args: " + to_string(args.size()) + "]";
    }
    llvm::Value* codegen(llvm::IRBuilder<>* builder,
                         llvm::LLVMContext* context,
                         llvm::BasicBlock* entryBlock,
                         map<string, llvm::Value*>* namedValues,
                         llvm::Module* module,
                         map<string, string>* objectTypes,
                         map<string, ClassData>* classes) override;
};


#endif //STELLAR_ASTCLASSINSTANTIATION_H
