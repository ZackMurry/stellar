//
// Created by zack on 7/29/21.
//

#include "include/ASTNullCheckExpression.h"

llvm::Value * ASTNullCheckExpression::codegen(llvm::IRBuilder<> *builder,
                                              llvm::LLVMContext *context,
                                              llvm::BasicBlock *entryBlock,
                                              std::map<std::string, llvm::Value *> *namedValues,
                                              llvm::Module *module,
                                              map<string, string> *objectTypes,
                                              map<string, ClassData> *classes) {
    return builder->CreateIsNull(value->codegen(builder, context, entryBlock, namedValues, module, objectTypes, classes), "nullchktmp");
}
