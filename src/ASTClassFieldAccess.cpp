//
// Created by zack on 7/15/21.
//

#include "include/ASTClassFieldAccess.h"

llvm::Value * ASTClassFieldAccess::codegen(llvm::IRBuilder<> *builder, llvm::LLVMContext *context,
                                           llvm::BasicBlock *entryBlock, map<string, llvm::Value *> *namedValues,
                                           llvm::Module *module) {
    vector<llvm::Value*> elementIndex = { llvm::ConstantInt::get(*context, llvm::APInt(32, 0)), llvm::ConstantInt::get(*context, llvm::APInt(32, fieldNumber)) };
    llvm::Value* gep = builder->CreateGEP(builder->CreateLoad(namedValues->at(identifier)), elementIndex);
    return builder->CreateLoad(gep);
}
