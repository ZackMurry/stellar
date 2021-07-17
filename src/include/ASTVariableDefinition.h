//
// Created by zack on 7/9/21.
//

#include <string>
#include "parser.h"
#ifndef STELLAR_ASTVARIABLEDEFINITION_H
#define STELLAR_ASTVARIABLEDEFINITION_H


class ASTVariableDefinition : public ASTNode {
    std::string name;
    VariableType type;
public:
    ASTVariableDefinition(std::string name, VariableType type) : name(move(name)), type(type) {}
    std::string toString() override {
        return "[VAR_DEF: " + name + " " + std::to_string(type) + "]";
    }
    llvm::Value *codegen(llvm::IRBuilder<>* builder,
                         llvm::LLVMContext* context,
                         llvm::BasicBlock* entryBlock,
                         map<string, llvm::Value*>* namedValues,
                         llvm::Module* module,
                         map<string, string>* objectTypes,
                         map<string, ClassData>* classes) override;
    VariableType getType() { return type; };
    std::string getName() { return name; };
};


#endif //STELLAR_ASTVARIABLEDEFINITION_H
