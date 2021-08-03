//
// Created by zack on 8/1/21.
//

#include "include/ASTNotExpression.h"

llvm::Value* ASTNotExpression::codegen(CodegenData data) {
    return data.builder->CreateNot(value->codegen(data));
}
