//
// Created by zack on 7/14/21.
//

#include "include/ASTStringExpression.h"

llvm::Value* ASTStringExpression::codegen(llvm::IRBuilder<> *builder,
                                          llvm::LLVMContext *context,
                                          llvm::BasicBlock *entryBlock,
                                          map<string, llvm::Value *> *namedValues,
                                          llvm::Module *module,
                                          map<string, string>* objectTypes,
                                          map<string, ClassData>* classes) {
    return builder->CreateGlobalStringPtr(llvm::StringRef(val));
}
