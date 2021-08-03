//
// Created by zack on 7/9/21.
//

#include "include/ASTVariableDeclaration.h"

llvm::Value* ASTVariableDeclaration::codegen(llvm::IRBuilder<>* builder,
                                             llvm::LLVMContext* context,
                                             llvm::BasicBlock* entryBlock,
                                             map<string, llvm::Value*>* namedValues,
                                             llvm::Module* module,
                                             map<string, string>* objectTypes,
                                             map<string, ClassData>* classes) {
    if (type.type == "s") {
        auto val = value->codegen(builder, context, entryBlock, namedValues, module, objectTypes, classes);
        namedValues->insert({ name, val });
        return val;
    } else {
        auto genericType = convertVariableTypeToString(type);
        int ivt = getPrimitiveVariableTypeFromString(genericType);
        llvm::Type *llvmType;
        if (ivt != -1) {
            llvmType = getLLVMTypeByPrimitiveVariableType((PrimitiveVariableType) ivt, context);
        } else if (classes->count(genericType)) {
            llvmType = llvm::PointerType::getUnqual(classes->at(genericType).type);
        } else {
            cerr << "Error: unknown variable type " << genericType << endl;
            exit(EXIT_FAILURE);
        }
        llvm::AllocaInst *alloca = builder->CreateAlloca(llvmType, nullptr, name);
        builder->CreateStore(
                value->codegen(builder, context, entryBlock, namedValues, module, objectTypes, classes), alloca);
        namedValues->insert({name, alloca});
        if (classes->count(genericType)) {
            objectTypes->insert({ name, genericType });
        }
        return alloca;
    }
}

