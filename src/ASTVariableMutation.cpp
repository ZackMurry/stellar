//
// Created by zack on 7/26/21.
//

#include "include/ASTVariableMutation.h"

llvm::Value* ASTVariableMutation::codegen(CodegenData data) {
    cout << "Getting named val" << endl;
    if (!data.namedValues->count(name)) {
        cerr << "Error: illegal use of undeclared variable '" << name << "'" << endl;
        exit(EXIT_FAILURE);
    }
    llvm::Value* var = data.namedValues->at(name);
    auto* load = data.builder->CreateLoad(var); // Load before the variable is changed
    llvm::Value* modified;
    auto* rhs = change->codegen(data);
    if (load->getType()->isFloatingPointTy() && rhs->getType()->isIntegerTy()) {
        cout << "Cast" << endl;
        rhs = data.builder->CreateSIToFP(rhs, load->getType(), "conv");
    }

    if (load->getType()->isFloatingPointTy()) {
        if (mutationType == MUTATION_TYPE_ADD) {
            modified = data.builder->CreateFAdd(load, rhs);
        } else if (mutationType == MUTATION_TYPE_SUB){
            modified = data.builder->CreateFSub(load, rhs);
        } else if (mutationType == MUTATION_TYPE_MUL) {
            modified = data.builder->CreateFMul(load, rhs);
        } else {
            modified = data.builder->CreateFDiv(load, rhs);
        }
    } else {
        if (mutationType == MUTATION_TYPE_ADD) {
            modified = data.builder->CreateAdd(load, rhs);
        } else if (mutationType == MUTATION_TYPE_SUB){
            modified = data.builder->CreateSub(load, rhs);
        } else if (mutationType == MUTATION_TYPE_MUL) {
            modified = data.builder->CreateMul(load, rhs);
        } else {
            modified = data.builder->CreateSDiv(load, rhs);
        }
    }
    data.builder->CreateStore(modified, var);
    if (mutationPosition == MUTATE_BEFORE) {
        return modified;
    }
    return load;
}
