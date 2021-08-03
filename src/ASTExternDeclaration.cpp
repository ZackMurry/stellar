//
// Created by zack on 7/9/21.
//

#include "include/ASTExternDeclaration.h"
#include <iostream>

using namespace std;

llvm::Value* ASTExternDeclaration::codegen(CodegenData data) {
    vector<llvm::Type*> llvmArgTypes;
    for (const auto& at : argTypes) {
        llvm::Type* llvmType;
        int ivt = getPrimitiveVariableTypeFromString(at);
        if (ivt != -1) {
            llvmType = getLLVMTypeByPrimitiveVariableType((PrimitiveVariableType) ivt, data.context);
        } else if (data.classes->count(at)) {
            llvmType = llvm::PointerType::getUnqual(data.classes->at(at).type);
        } else {
            cerr << "Error: unknown type " << at << endl;
            exit(EXIT_FAILURE);
        }
        if (llvmType == nullptr) {
            cerr << "Error mapping variable type to LLVM type" << endl;
            exit(EXIT_FAILURE);
        }
        llvmArgTypes.push_back(llvmType);
    }
    llvm::FunctionType* ft = llvm::FunctionType::get(getLLVMTypeByPrimitiveVariableType(returnType, data.context), llvmArgTypes, isVarArgs);
    llvm::Function* func = llvm::Function::Create(ft, llvm::Function::ExternalLinkage, name, *data.module);
    return func;
}

