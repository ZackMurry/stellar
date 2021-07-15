//
// Created by zack on 7/15/21.
//

#include "include/ASTClassFieldStore.h"

llvm::Value * ASTClassFieldStore::codegen(llvm::IRBuilder<> *builder, llvm::LLVMContext *context,
                                          llvm::BasicBlock *entryBlock, map<string, llvm::Value *> *namedValues,
                                          llvm::Module *module) {
    vector<llvm::Value*> elementIndex = { llvm::ConstantInt::get(*context, llvm::APInt(32, 0)), llvm::ConstantInt::get(*context, llvm::APInt(32, fieldNumber)) };
    cout << "el index: " << fieldNumber << endl;
    llvm::Value* gep = builder->CreateStructGEP(builder->CreateLoad(namedValues->at(identifier)), fieldNumber);
    return builder->CreateStore(value->codegen(builder, context, entryBlock, namedValues, module), gep);
}
