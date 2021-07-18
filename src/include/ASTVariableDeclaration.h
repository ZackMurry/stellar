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
    string type;
    ASTNode* value;
public:
    ASTVariableDeclaration(string name, string type, ASTNode* value) : name(move(name)), type(move(type)), value(value) {}
    string toString() override {
        return "[VAR_DECL: " + name + " " + type + " " + value->toString() + "]";
    }
    llvm::Value *codegen(llvm::IRBuilder<>* builder,
                         llvm::LLVMContext* context,
                         llvm::BasicBlock* entryBlock,
                         map<string, llvm::Value*>* namedValues,
                         llvm::Module* module,
                         map<string, string>* objectTypes,
                         map<string, ClassData>* classes) override;
};


#endif //STELLAR_ASTVARIABLEDECLARATION_H
