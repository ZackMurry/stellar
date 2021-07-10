//
// Created by zack on 7/9/21.
//

#include "include/ASTArrayAccess.h"

llvm::Value* ASTArrayAccess::codegen(llvm::IRBuilder<>* builder,
                                     llvm::LLVMContext* context,
                                     llvm::BasicBlock* entryBlock,
                                     map<string, llvm::Value*> namedValues,
                                     llvm::Module* module) {
    auto* gep = builder->CreateInBoundsGEP(namedValues[name], index->codegen(builder, context, entryBlock, namedValues, module), "acctmp");
    gep->mutateType(getLLVMPtrTypeByType(namedValues[name]->getType()->getPointerElementType(), context));
    return builder->CreateLoad(gep, "loadtmp");
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
    cerr << "Error: unknown type" << endl;
    exit(EXIT_FAILURE);
}
