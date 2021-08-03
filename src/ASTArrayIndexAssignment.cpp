//
// Created by zack on 7/9/21.
//

#include "include/ASTArrayIndexAssignment.h"

llvm::Value* ASTArrayIndexAssignment::codegen(CodegenData data) {
    llvm::Value* ref = data.builder->CreateInBoundsGEP(data.namedValues->at(name), llvm::ArrayRef<llvm::Value*>(index->codegen(data)), "acctmp");
    return data.builder->CreateStore(value->codegen(data), ref);
}
