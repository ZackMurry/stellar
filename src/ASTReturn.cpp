//
// Created by zack on 7/9/21.
//

#include "include/ASTReturn.h"

llvm::Value* ASTReturn::codegen(CodegenData data) {
    if (exp) {
        llvm::Value* val = exp->codegen(data);
        return data.builder->CreateRet(val);
    } else { // Void
        return data.builder->CreateRetVoid();
    }
}
