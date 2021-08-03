//
// Created by zack on 7/9/21.
//

#include "include/ASTExternDeclaration.h"
#include <iostream>

using namespace std;

llvm::Value* ASTExternDeclaration::codegen(llvm::IRBuilder<>* builder,
                                           llvm::LLVMContext* context,
                                           llvm::BasicBlock* entryBlock,
                                           map<string, llvm::Value*>* namedValues,
                                           llvm::Module* module,
                                           map<string, string>* objectTypes,
                                           map<string, ClassData>* classes) {
    vector<llvm::Type*> llvmArgTypes;
    for (const auto& at : argTypes) {
        llvm::Type* llvmType;
        int ivt = getPrimitiveVariableTypeFromString(at);
        if (ivt != -1) {
            llvmType = getLLVMTypeByPrimitiveVariableType((PrimitiveVariableType) ivt, context);
        } else if (classes->count(at)) {
            llvmType = llvm::PointerType::getUnqual(classes->at(at).type);
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
    llvm::FunctionType* ft = llvm::FunctionType::get(getLLVMTypeByPrimitiveVariableType(returnType, context), llvmArgTypes, isVarArgs);
    llvm::Function* func = llvm::Function::Create(ft, llvm::Function::ExternalLinkage, name, *module);
    return func;
}

