//
// Created by zack on 8/1/21.
//

#include "include/ASTNotExpression.h"

llvm::Value * ASTNotExpression::codegen(llvm::IRBuilder<> *builder,
                                        llvm::LLVMContext *context,
                                        llvm::BasicBlock *entryBlock,
                                        map<string, llvm::Value *> *namedValues,
                                        llvm::Module *module,
                                        map<string, string> *objectTypes,
                                        map<string, ClassData> *classes) {
    return builder->CreateNot(value->codegen(builder, context, entryBlock, namedValues, module, objectTypes, classes));
}
