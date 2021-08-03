//
// Created by zack on 7/9/21.
//

#include <string>
#include "parser.h"

#ifndef STELLAR_ASTVARIABLEDECLARATION_H
#define STELLAR_ASTVARIABLEDECLARATION_H

using namespace std;

class ASTVariableDeclaration : public ASTNode {
public:
    string name;
    VariableType type;
    ASTNode* value;
    ASTVariableDeclaration(string name, VariableType type, ASTNode* value) : name(move(name)), type(move(type)), value(value) {}
    string toString() override {
        return "[VAR_DECL: " + name + " " + type.type + " " + value->toString() + "]";
    }
    llvm::Value *codegen(llvm::IRBuilder<>* builder,
                         llvm::LLVMContext* context,
                         llvm::BasicBlock* entryBlock,
                         map<string, llvm::Value*>* namedValues,
                         llvm::Module* module,
                         map<string, string>* objectTypes,
                         map<string, ClassData>* classes) override;
    ASTNodeType getType() override {
        return AST_VARIABLE_DECLARATION;
    }
};


#endif //STELLAR_ASTVARIABLEDECLARATION_H
