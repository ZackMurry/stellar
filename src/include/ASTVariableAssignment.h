//
// Created by zack on 7/9/21.
//

#include "parser.h"
#ifndef STELLAR_ASTVARIABLEASSIGNMENT_H
#define STELLAR_ASTVARIABLEASSIGNMENT_H

using namespace std;

class ASTVariableAssignment : public ASTNode {
public:
    string name;
    ASTNode* value;
    ASTVariableAssignment(string name, ASTNode* value) : name(move(name)), value(value) {};
    string toString() override {
        return "[VAR_ASSIGN: " + name + " " + value->toString() + "]";
    }
    llvm::Value *codegen(CodegenData data) override;
    ASTNodeType getType() override {
        return AST_VARIABLE_ASSIGNMENT;
    }
};



#endif //STELLAR_ASTVARIABLEASSIGNMENT_H
