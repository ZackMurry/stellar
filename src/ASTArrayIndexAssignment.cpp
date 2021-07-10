//
// Created by zack on 7/9/21.
//

#include "include/ASTArrayIndexAssignment.h"

llvm::Value* ASTArrayIndexAssignment::codegen(llvm::IRBuilder<>* builder,
                                              llvm::LLVMContext* context,
                                              llvm::BasicBlock* entryBlock,
                                              map<string, llvm::Value*> namedValues,
                                              llvm::Module* module) {
    llvm::Value* ref = builder->CreateInBoundsGEP(namedValues[name], llvm::ArrayRef<llvm::Value*>(index->codegen(builder, context, entryBlock, namedValues, module)), "acctmp");
    return builder->CreateStore(value->codegen(builder, context, entryBlock, namedValues, module), ref);
}
