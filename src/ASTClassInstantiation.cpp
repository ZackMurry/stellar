//
// Created by zack on 7/15/21.
//

#include "include/ASTClassInstantiation.h"

llvm::Value* ASTClassInstantiation::codegen(llvm::IRBuilder<> *builder, llvm::LLVMContext *context,
                                            llvm::BasicBlock *entryBlock, map<string, llvm::Value *> *namedValues,
                                            llvm::Module *module) {
    auto alloca = builder->CreateAlloca(llvm::PointerType::getUnqual(type), nullptr, name);
//    auto inst = llvm::CallInst::CreateMalloc(
//            builder->GetInsertBlock(),
//            llvm::Type::getInt64PtrTy(*context),
//            type,
//            llvm::ConstantExpr::getSizeOf(type),
//            nullptr,
//            nullptr,
//            name);
//    builder->CreateStore(builder->Insert(inst), alloca);
    namedValues->insert({ name, alloca });
    return alloca;
}
