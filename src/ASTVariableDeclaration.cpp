//
// Created by zack on 7/9/21.
//

#include "include/ASTVariableDeclaration.h"

llvm::Value* ASTVariableDeclaration::codegen(llvm::IRBuilder<>* builder,
                                             llvm::LLVMContext* context,
                                             llvm::BasicBlock* entryBlock,
                                             map<string, llvm::Value*>* namedValues,
                                             llvm::Module* module) {
    llvm::Type* llvmType = getLLVMTypeByVariableType(type, context);
    llvm::AllocaInst* alloca = builder->CreateAlloca(llvmType, nullptr, name);
    builder->CreateStore(value->codegen(builder, context, entryBlock, namedValues, module), alloca);
    namedValues->insert({ name, alloca });
    return alloca;
}

