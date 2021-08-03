//
// Created by zack on 7/9/21.
//

#include <llvm/IR/Instructions.h>
#include <llvm/IR/IRBuilder.h>
#include "include/ASTVariableDefinition.h"

llvm::Value* ASTVariableDefinition::codegen(llvm::IRBuilder<>* builder,
                                            llvm::LLVMContext* context,
                                            llvm::BasicBlock* entryBlock,
                                            std::map<std::string, llvm::Value*>* namedValues,
                                            llvm::Module* module,
                                            map<string, string>* objectTypes,
                                            map<string, ClassData>* classes) {
    auto genericType = convertVariableTypeToString(type);
    int ivt = getPrimitiveVariableTypeFromString(genericType);
    llvm::Type* llvmType;
    if (ivt != -1) {
        llvmType = getLLVMTypeByPrimitiveVariableType((PrimitiveVariableType) ivt, context);
    } else if (classes->count(genericType)) {
        auto ptrTy = llvm::PointerType::getUnqual(classes->at(genericType).type);
        auto val = llvm::ConstantPointerNull::get(ptrTy);
        auto alloca = builder->CreateAlloca(ptrTy, nullptr, name);
        builder->CreateStore(val, alloca);
        namedValues->insert({ name, alloca });
        objectTypes->insert({ name, genericType });
        return alloca;
    } else {
        cerr << "Error: unknown variable type " << genericType << endl;
        exit(EXIT_FAILURE);
    }
    llvm::AllocaInst* alloca = builder->CreateAlloca(llvmType, nullptr, name);
    namedValues->insert({ name, alloca });
    return alloca;
}
