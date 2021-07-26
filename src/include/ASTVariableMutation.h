//
// Created by zack on 7/26/21.
//

#include "parser.h"
#ifndef STELLAR_ASTVARIABLEMUTATION_H
#define STELLAR_ASTVARIABLEMUTATION_H

enum MutationType {
    MUTATION_TYPE_ADD,
    MUTATION_TYPE_SUB
};

enum MutationPosition {
    MUTATE_BEFORE,
    MUTATE_AFTER
};

class ASTVariableMutation : public ASTNode {
    string name;
    ASTNode* change;
    MutationType mutationType;
    MutationPosition mutationPosition;
public:
    ASTVariableMutation(string name, ASTNode* change, MutationType mutationType, MutationPosition mutationPosition) : name(move(name)), change(change), mutationType(mutationType), mutationPosition(mutationPosition) {}
    string toString() override {
        return "[VAR_MUT: " + name + " change:  " + change->toString() + " type: " + to_string(mutationType) + " pos: " +
                to_string(mutationPosition) + "]";
    }
    llvm::Value *codegen(llvm::IRBuilder<>* builder,
                         llvm::LLVMContext* context,
                         llvm::BasicBlock* entryBlock,
                         std::map<std::string, llvm::Value*>* namedValues,
                         llvm::Module* module,
                         map<string, string>* objectTypes,
                         map<string, ClassData>* classes) override;
};


#endif //STELLAR_ASTVARIABLEMUTATION_H
