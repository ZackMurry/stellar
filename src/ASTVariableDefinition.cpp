//
// Created by zack on 7/9/21.
//

#include <llvm/IR/Instructions.h>
#include <llvm/IR/IRBuilder.h>
#include "include/ASTVariableDefinition.h"

llvm::Value* ASTVariableDefinition::codegen(llvm::IRBuilder<>* builder,
                                            llvm::LLVMContext* context,
                                            llvm::BasicBlock* entryBlock,
                                            std::map<std::string, llvm::Value*>* namedValues,
                                            llvm::Module* module) {
    llvm::Type* llvmType = getLLVMTypeByVariableType(type, context);
    llvm::AllocaInst* alloca = builder->CreateAlloca(llvmType, nullptr, name);
    namedValues->insert({ name, alloca });
    return alloca;
}
