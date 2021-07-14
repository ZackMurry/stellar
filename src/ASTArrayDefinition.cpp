//
// Created by zack on 7/9/21.
//

#include "include/ASTArrayDefinition.h"

llvm::Value* ASTArrayDefinition::codegen(llvm::IRBuilder<>* builder,
                                         llvm::LLVMContext* context,
                                         llvm::BasicBlock* entryBlock,
                                         map<string, llvm::Value*>* namedValues,
                                         llvm::Module* module) {
    llvm::Type* llvmElType = getLLVMTypeByVariableType(elementType, context);
    llvm::AllocaInst* alloca = builder->CreateAlloca(llvmElType, length->codegen(builder, context, entryBlock, namedValues, module), name);
    namedValues->insert({ name, alloca });
    return alloca;
}

