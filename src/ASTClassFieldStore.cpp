//
// Created by zack on 7/15/21.
//

#include "include/ASTClassFieldStore.h"

llvm::Value * ASTClassFieldStore::codegen(llvm::IRBuilder<> *builder, llvm::LLVMContext *context,
                                          llvm::BasicBlock *entryBlock, map<string, llvm::Value *> *namedValues,
                                          llvm::Module *module) {
    // todo compute fieldNumber here instead of in parser
    vector<llvm::Value*> elementIndex = { llvm::ConstantInt::get(*context, llvm::APInt(32, 0)), llvm::ConstantInt::get(*context, llvm::APInt(32, fieldNumber)) };
    cout << "el index: " << fieldNumber << endl;
    llvm::Value* gep = builder->CreateGEP(builder->CreateLoad(namedValues->at(identifier)), elementIndex);
    cout << "gep created" << endl;
    return builder->CreateStore(value->codegen(builder, context, entryBlock, namedValues, module), gep);
}
