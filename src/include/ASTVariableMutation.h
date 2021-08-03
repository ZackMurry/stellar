//
// Created by zack on 7/26/21.
//

#include "parser.h"
#ifndef STELLAR_ASTVARIABLEMUTATION_H
#define STELLAR_ASTVARIABLEMUTATION_H

enum MutationType {
    MUTATION_TYPE_ADD,
    MUTATION_TYPE_SUB,
    MUTATION_TYPE_MUL,
    MUTATION_TYPE_DIV
};

enum MutationPosition {
    MUTATE_BEFORE,
    MUTATE_AFTER
};

class ASTVariableMutation : public ASTNode {
public:
    string name;
    ASTNode* change;
    MutationType mutationType;
    MutationPosition mutationPosition;
    ASTVariableMutation(string name, ASTNode* change, MutationType mutationType, MutationPosition mutationPosition) : name(move(name)), change(change), mutationType(mutationType), mutationPosition(mutationPosition) {}
    string toString() override {
        return "[VAR_MUT: " + name + " change:  " + change->toString() + " type: " + to_string(mutationType) + " pos: " +
                to_string(mutationPosition) + "]";
    }
    llvm::Value *codegen(CodegenData data) override;
    ASTNodeType getType() override {
        return AST_VARIABLE_MUTATION;
    }
};


#endif //STELLAR_ASTVARIABLEMUTATION_H
