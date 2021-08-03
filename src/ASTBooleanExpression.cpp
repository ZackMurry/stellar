//
// Created by zack on 7/29/21.
//

#include "include/ASTBooleanExpression.h"

llvm::Value * ASTBooleanExpression::codegen(CodegenData data) {
    return llvm::ConstantInt::get(llvm::Type::getInt1Ty(*data.context), value);
}
