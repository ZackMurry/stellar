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
public:
    ASTClassInstantiation(string className, string identifier) : className(move(className)), identifier(move(identifier)) {}
    string toString() override {
        return "[CLASS_INST: type: " + className + " name: " + identifier + "]";
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
