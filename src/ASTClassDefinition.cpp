//
// Created by zack on 7/15/21.
//

#include "include/ASTClassDefinition.h"

llvm::Value* ASTClassDefinition::codegen(llvm::IRBuilder<> *builder, llvm::LLVMContext *context,
                                         llvm::BasicBlock *entryBlock, map<string, llvm::Value *> *namedValues,
                                         llvm::Module *module) {
}
