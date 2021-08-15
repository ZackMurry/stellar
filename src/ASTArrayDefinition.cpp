//
// Created by zack on 7/9/21.
//

#include "include/ASTArrayDefinition.h"

llvm::Value* ASTArrayDefinition::codegen(CodegenData data) {
    llvm::Type* llvmElType;
    int ivt = getPrimitiveVariableTypeFromString(elementType);
    if (ivt != -1) {
        llvmElType = getLLVMTypeByPrimitiveVariableType((PrimitiveVariableType) ivt, data.context);
    } else if (data.classes->count(elementType)) {
        llvmElType = llvm::PointerType::getUnqual(data.classes->at(elementType).type);
    } else {
        cerr << "Error: unknown array element type " << elementType << endl;
        exit(EXIT_FAILURE);
    }
    auto alloca = data.builder->CreateAlloca(llvm::PointerType::getUnqual(llvmElType), nullptr, "new_arr");
    auto inst = llvm::CallInst::CreateMalloc(
            data.builder->GetInsertBlock(),
            llvm::Type::getInt64PtrTy(*data.context),
            llvmElType,
            llvm::ConstantExpr::getSizeOf(llvmElType),
            length->codegen(data),
            nullptr,
            "arr_malloc");
    data.builder->CreateStore(data.builder->Insert(inst), alloca);
//    auto allocaPtr = data.builder->CreateAlloca(llvm::PointerType::getUnqual(llvmElType), nullptr, name);
//    data.builder->CreateStore(alloca, allocaPtr);
//    data.namedValues->insert({ name, allocaPtr });
    data.namedValues->insert({ name, alloca });
    return inst;
}

