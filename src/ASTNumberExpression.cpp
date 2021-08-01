//
// Created by zack on 7/9/21.
//

#include "include/ASTNumberExpression.h"
#include <iostream>

llvm::Value* ASTNumberExpression::codegen(llvm::IRBuilder<>* builder,
                                          llvm::LLVMContext* context,
                                          llvm::BasicBlock* entryBlock,
                                          map<string, llvm::Value*>* namedValues,
                                          llvm::Module* module,
                                          map<string, string>* objectTypes,
                                          map<string, ClassData>* classes) {
    if (type == VARIABLE_TYPE_I32) {
        return llvm::ConstantInt::get(llvm::Type::getInt32Ty(*context), val, 10);
    } else if (type == VARIABLE_TYPE_F) {
        return llvm::ConstantFP::get(llvm::Type::getFloatTy(*context), val);
    } else if (type == VARIABLE_TYPE_D) {
        return llvm::ConstantFP::get(llvm::Type::getDoubleTy(*context), val);
    } else if (type == VARIABLE_TYPE_I8) {
        return llvm::ConstantInt::get(llvm::Type::getInt8Ty(*context), val, 10);
    } else if (type == VARIABLE_TYPE_I16) {
        cout << "i16 type" << endl;
        return llvm::ConstantInt::get(llvm::Type::getInt16Ty(*context), val, 10);
    } else if (type == VARIABLE_TYPE_I64) {
        return llvm::ConstantInt::get(llvm::Type::getInt64Ty(*context), val, 10);
    } else {
        cerr << "Error: unimplemented number type " << type << endl;
        exit(EXIT_FAILURE);
    }
}
