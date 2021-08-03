//
// Created by zack on 7/29/21.
//

#include "include/ASTNullCheckExpression.h"

llvm::Value* ASTNullCheckExpression::codegen(CodegenData data) {
    return data.builder->CreateIsNull(value->codegen(data), "nullchktmp");
}
