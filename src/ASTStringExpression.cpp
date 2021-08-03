//
// Created by zack on 7/14/21.
//

#include "include/ASTStringExpression.h"

llvm::Value* ASTStringExpression::codegen(CodegenData data) {
    return data.builder->CreateGlobalStringPtr(llvm::StringRef(val));
}
