//
// Created by zack on 7/9/21.
//

#include <llvm/IR/Instructions.h>
#include "include/ASTVariableDefinition.h"

llvm::Value* ASTVariableDefinition::codegen(CodegenData data) {
    if (data.generics->count(type.type)) {
        type = data.generics->at(type.type);
    }
    auto genericType = convertVariableTypeToString(type);
    int ivt = getPrimitiveVariableTypeFromString(genericType);
    llvm::Type* llvmType;
    if (ivt != -1) {
        llvmType = getLLVMTypeByPrimitiveVariableType((PrimitiveVariableType) ivt, data.context);
    } else if (data.classes->count(genericType)) {
        auto ptrTy = llvm::PointerType::getUnqual(data.classes->at(genericType).type);
        auto val = llvm::ConstantPointerNull::get(ptrTy);
        auto alloca = data.builder->CreateAlloca(ptrTy, nullptr, name);
        data.builder->CreateStore(val, alloca);
        data.namedValues->insert({ name, alloca });
        data.objectTypes->insert({ name, genericType });
        return alloca;
    } else {
        cerr << "Error: unknown variable type " << genericType << endl;
        exit(EXIT_FAILURE);
    }
    llvm::AllocaInst* alloca = data.builder->CreateAlloca(llvmType, nullptr, name);
    data.namedValues->insert({ name, alloca });
    return alloca;
}
