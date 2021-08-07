//
// Created by zack on 7/9/21.
//

#include "include/ASTNumberExpression.h"
#include <iostream>

llvm::Value* ASTNumberExpression::codegen(CodegenData data) {
    if (type == VARIABLE_TYPE_I32) {
        return llvm::ConstantInt::get(llvm::Type::getInt32Ty(*data.context), val, 10);
    } else if (type == VARIABLE_TYPE_F32) {
        return llvm::ConstantFP::get(llvm::Type::getFloatTy(*data.context), val);
    } else if (type == VARIABLE_TYPE_F64) {
        return llvm::ConstantFP::get(llvm::Type::getDoubleTy(*data.context), val);
    } else if (type == VARIABLE_TYPE_I8) {
        return llvm::ConstantInt::get(llvm::Type::getInt8Ty(*data.context), val, 10);
    } else if (type == VARIABLE_TYPE_I16) {
        cout << "i16 type" << endl;
        return llvm::ConstantInt::get(llvm::Type::getInt16Ty(*data.context), val, 10);
    } else if (type == VARIABLE_TYPE_I64) {
        return llvm::ConstantInt::get(llvm::Type::getInt64Ty(*data.context), val, 10);
    } else {
        cerr << "Error: unimplemented number type " << type << endl;
        exit(EXIT_FAILURE);
    }
}
