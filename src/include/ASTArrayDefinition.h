//
// Created by zack on 7/9/21.
//

#include "parser.h"
#ifndef STELLAR_ASTARRAYDEFINITION_H
#define STELLAR_ASTARRAYDEFINITION_H

using namespace std;

class ASTArrayDefinition : public ASTNode {
public:
    string name;
    string elementType;
    ASTNode* length;
    ASTArrayDefinition(string name, string elementType, ASTNode* length) : name(move(name)), elementType(move(elementType)), length(length) {}
    string toString() override {
        return "[ARR_DEF: " + name + " " + elementType + " size: " + length->toString() + "]";
    }
    llvm::Value* codegen(CodegenData data) override;
    ASTNodeType getType() override {
        return AST_ARRAY_DEFINITION;
    }
};


#endif //STELLAR_ASTARRAYDEFINITION_H
