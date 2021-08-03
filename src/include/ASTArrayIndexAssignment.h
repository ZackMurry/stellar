//
// Created by zack on 7/9/21.
//

#include "parser.h"
#ifndef STELLAR_ASTARRAYINDEXASSIGNMENT_H
#define STELLAR_ASTARRAYINDEXASSIGNMENT_H

class ASTArrayIndexAssignment : public ASTNode {
public:
    string name;
    ASTNode* index;
    ASTNode* value;
    ASTArrayIndexAssignment(string name, ASTNode* index, ASTNode* value) : name(move(name)), index(index), value(value) {}
    string toString() override {
        return "[ARR_ASSIGN: " + name + " at " + index->toString() + " to " + value->toString() + "]";
    }
    llvm::Value* codegen(CodegenData data) override;
    ASTNodeType getType() override {
        return AST_ARRAY_INDEX_ASSIGNMENT;
    }
};

#endif //STELLAR_ASTARRAYINDEXASSIGNMENT_H
