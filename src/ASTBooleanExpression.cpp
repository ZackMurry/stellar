//
// Created by zack on 7/29/21.
//

#include "include/ASTBooleanExpression.h"

llvm::Value * ASTBooleanExpression::codegen(llvm::IRBuilder<> *builder,
                                            llvm::LLVMContext *context,
                                            llvm::BasicBlock *entryBlock,
                                            std::map<std::string, llvm::Value *> *namedValues,
                                            llvm::Module *module,
                                            map<string, string> *objectTypes,
                                            map<string, ClassData> *classes) {
    return llvm::ConstantInt::get(llvm::Type::getInt1Ty(*context), value);
}
