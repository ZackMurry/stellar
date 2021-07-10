//
// Created by zack on 7/9/21.
//

#include "include/ASTReturn.h"

llvm::Value* ASTReturn::codegen(llvm::IRBuilder<>* builder,
                                llvm::LLVMContext* context,
                                llvm::BasicBlock* entryBlock,
                                map<string, llvm::Value*> namedValues,
                                llvm::Module* module) {
    if (exp) {
        llvm::Value* val = exp->codegen(builder, context, entryBlock, namedValues, module);
        return builder->CreateRet(val);
    } else { // Void
        return builder->CreateRetVoid();
    }
}
