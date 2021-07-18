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
    if (type == "s") {
        auto val = value->codegen(builder, context, entryBlock, namedValues, module, objectTypes, classes);
        namedValues->insert({ name, val });
        return val;
    } else {
        int ivt = getVariableTypeFromString(type);
        llvm::Type *llvmType;
        if (ivt != -1) {
            llvmType = getLLVMTypeByVariableType((VariableType) ivt, context);
        } else if (classes->count(type)) {
            llvmType = llvm::PointerType::get(classes->at(type).type, 0);
        } else {
            cerr << "Error: unknown variable type " << type << endl;
            exit(EXIT_FAILURE);
        }
        llvm::AllocaInst *alloca = builder->CreateAlloca(llvmType, nullptr, name);
        builder->CreateStore(
                value->codegen(builder, context, entryBlock, namedValues, module, objectTypes, classes), alloca);
        namedValues->insert({name, alloca});
        if (classes->count(type)) {
            objectTypes->insert({ name, type });
        }
        return alloca;
    }
}

