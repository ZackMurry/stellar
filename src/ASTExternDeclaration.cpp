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
    vector<llvm::Type*> argTypes;
    for (const auto& arg : args) {
        llvm::Type* llvmType;// = getLLVMTypeByVariableType(arg->getType(), context);
        int ivt = getVariableTypeFromString(arg->getType());
        if (ivt != -1) {
            llvmType = getLLVMTypeByVariableType((VariableType) ivt, context);
        } else if (classes->count(arg->getType())) {
            llvmType = classes->at(arg->getType()).type;
        } else {
            cerr << "Error: unknown type " << arg->getType() << endl;
            exit(EXIT_FAILURE);
        }
        if (llvmType == nullptr) {
            cerr << "Error mapping variable type to LLVM type" << endl;
            exit(EXIT_FAILURE);
        }
        argTypes.push_back(llvmType);
    }
    llvm::FunctionType* ft = llvm::FunctionType::get(getLLVMTypeByVariableType(returnType, context), argTypes, false);
    llvm::Function* func = llvm::Function::Create(ft, llvm::Function::ExternalLinkage, name, *module);
    unsigned index = 0;
    for (auto &arg : func->args()) {
        arg.setName(args[index++]->getName());
    }
    return func;
}

