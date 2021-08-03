//
// Created by zack on 7/15/21.
//

#include "parser.h"
#include <string>
#ifndef STELLAR_ASTCLASSINSTANTIATION_H
#define STELLAR_ASTCLASSINSTANTIATION_H

using namespace std;

class ASTClassInstantiation : public ASTNode {
public:
    string className;
    string identifier;
    vector<ASTNode*> args;
    vector<VariableType> genericTypes;
    ASTClassInstantiation(string className, string identifier, vector<ASTNode*> args, vector<VariableType> genericTypes) : className(move(className)), identifier(move(identifier)), args(move(args)), genericTypes(move(genericTypes)) {}
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
    ASTNodeType getType() override {
        return AST_CLASS_INSTANTIATION;
    }
};


#endif //STELLAR_ASTCLASSINSTANTIATION_H
