//
// Created by zack on 7/9/21.
//

#include <string>
#include "parser.h"

#ifndef STELLAR_ASTVARIABLEDECLARATION_H
#define STELLAR_ASTVARIABLEDECLARATION_H

using namespace std;

class ASTVariableDeclaration : public ASTNode {
    std::string name;
    VariableType type;
    ASTNode* value;
public:
    ASTVariableDeclaration(string name, VariableType type, ASTNode* value) : name(move(name)), type(type), value(value) {}
    string toString() override {
        return "[VAR_DECL: " + name + " " + to_string(type) + " " + value->toString() + "]";
    }
    llvm::Value *codegen(llvm::IRBuilder<>* builder,
                         llvm::LLVMContext* context,
                         llvm::BasicBlock* entryBlock,
                         map<string, llvm::Value*> namedValues,
                         llvm::Module* module) override;
};


#endif //STELLAR_ASTVARIABLEDECLARATION_H
