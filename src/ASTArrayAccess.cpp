//
// Created by zack on 7/9/21.
//

#include "include/ASTArrayAccess.h"

llvm::Value* ASTArrayAccess::codegen(CodegenData data) {
    if (!data.namedValues->count(name)) {
        cerr << "Error: undeclared variable " << name << endl;
        exit(EXIT_FAILURE);
    }
    auto* gep = data.builder->CreateInBoundsGEP(data.namedValues->at(name), index->codegen(data), "acctmp");
    if (!gep->getType()->isPointerTy()) {
        gep->mutateType(getLLVMPtrTypeByType(data.namedValues->at(name)->getType()->getPointerElementType(), data.context));
    }
    return data.builder->CreateLoad(gep, "loadtmp");
}

llvm::Type* getLLVMPtrTypeByType(llvm::Type* type, llvm::LLVMContext* context) {
    if (type->isIntegerTy()) {
        if (type->isIntegerTy(8)) {
            cout << "type: i8" << endl;
            return llvm::Type::getInt8PtrTy(*context);
        }
        if (type->isIntegerTy(16)) {
            cout << "type: i16" << endl;
            return llvm::Type::getInt16PtrTy(*context);
        }
        if (type->isIntegerTy(32)) {
            cout << "type: i32" << endl;
            return llvm::Type::getInt32PtrTy(*context);
        }
        if (type->isIntegerTy(64)) {
            cout << "type: i64" << endl;
            return llvm::Type::getInt64PtrTy(*context);
        }
        cerr << "Error: unknown integer type" << endl;
        exit(EXIT_FAILURE);
    }
    if (type->isFloatingPointTy()) {
        if (type->isFloatTy()) {
            cout << "type: f" << endl;
            return llvm::Type::getFloatPtrTy(*context);
        }
        if (type->isDoubleTy()) {
            cout << "type; d" << endl;
            return llvm::Type::getDoublePtrTy(*context);
        }
    }
    cerr << "Error: unknown type to convert to pointer" << endl;
    exit(EXIT_FAILURE);
}
