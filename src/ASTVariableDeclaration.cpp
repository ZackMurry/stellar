//
// Created by zack on 7/9/21.
//

#include "include/ASTVariableDeclaration.h"

llvm::Value* ASTVariableDeclaration::codegen(CodegenData data) {
    if (data.generics->count(type.type)) {
        type = data.generics->at(type.type);
    }
    if (type.type == "s") {
        auto val = value->codegen(data);
        data.namedValues->insert({ name, val });
        return val;
    } else {
        auto genericType = convertVariableTypeToString(type);
        int ivt = getPrimitiveVariableTypeFromString(genericType);
        llvm::Type *llvmType;
        if (ivt != -1) {
            llvmType = getLLVMTypeByPrimitiveVariableType((PrimitiveVariableType) ivt, data.context);
        } else if (data.classes->count(genericType)) {
            llvmType = llvm::PointerType::getUnqual(data.classes->at(genericType).type);
        } else {
            cerr << "Error: unknown variable type " << genericType << endl;
            exit(EXIT_FAILURE);
        }
        llvm::AllocaInst *alloca = data.builder->CreateAlloca(llvmType, nullptr, name);
        data.builder->CreateStore(
                value->codegen(data), alloca);
        data.namedValues->insert({name, alloca});
        if (data.classes->count(genericType)) {
            data.objectTypes->insert({ name, genericType });
        }
        return alloca;
    }
}

