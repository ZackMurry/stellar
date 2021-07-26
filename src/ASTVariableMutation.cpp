//
// Created by zack on 7/26/21.
//

#include "include/ASTVariableMutation.h"

llvm::Value * ASTVariableMutation::codegen(llvm::IRBuilder<> *builder,
                                           llvm::LLVMContext *context,
                                           llvm::BasicBlock *entryBlock,
                                           std::map<std::string, llvm::Value *> *namedValues,
                                           llvm::Module *module,
                                           map<string, string> *objectTypes,
                                           map<string, ClassData> *classes) {
    cout << "Getting named val" << endl;
    if (!namedValues->count(name)) {
        cerr << "Error: illegal use of undeclared variable '" << name << "'" << endl;
        exit(EXIT_FAILURE);
    }
    llvm::Value* var = namedValues->at(name);
    auto* load = builder->CreateLoad(var); // Load before the variable is changed
    llvm::Value* modified;
    auto* rhs = change->codegen(builder, context, entryBlock, namedValues, module, objectTypes, classes);
    if (load->getType()->isFloatingPointTy() && rhs->getType()->isIntegerTy()) {
        cout << "Cast" << endl;
        rhs = builder->CreateSIToFP(rhs, load->getType(), "conv");
    }

    if (load->getType()->isFloatingPointTy()) {
        if (mutationType == MUTATION_TYPE_ADD) {
            modified = builder->CreateFAdd(load, rhs);
        } else {
            modified = builder->CreateFSub(load, rhs);
        }
    } else {
        if (mutationType == MUTATION_TYPE_ADD) {
            modified = builder->CreateAdd(load, rhs);
        } else {
            modified = builder->CreateSub(load, rhs);
        }
    }
    builder->CreateStore(modified, var);
    if (mutationPosition == MUTATE_BEFORE) {
        return modified;
    }
    return load;
}
