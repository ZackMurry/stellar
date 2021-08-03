//
// Created by zack on 7/9/21.
//

#include <string>
#include "parser.h"
#ifndef STELLAR_ASTVARIABLEDEFINITION_H
#define STELLAR_ASTVARIABLEDEFINITION_H


class ASTVariableDefinition : public ASTNode {
public:
    std::string name;
    VariableType type;
    ASTVariableDefinition(string name, VariableType type) : name(move(name)), type(move(type)) {}
    std::string toString() override {
        return "[VAR_DEF: " + name + " " + type.type + "]";
    }
    llvm::Value *codegen(CodegenData data) override;
    ASTNodeType getType() override {
        return AST_VARIABLE_DEFINITION;
    }
};


#endif //STELLAR_ASTVARIABLEDEFINITION_H
