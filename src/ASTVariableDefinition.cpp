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
    int ivt = getVariableTypeFromString(type);
    llvm::Type* llvmType;
    if (ivt != -1) {
        llvmType = getLLVMTypeByVariableType((VariableType) ivt, context);
    } else if (classes->count(type)) {
        llvmType = classes->at(type).type;
    } else {
        cerr << "Error: unknown variable type " << type << endl;
        exit(EXIT_FAILURE);
    }
    llvm::AllocaInst* alloca = builder->CreateAlloca(llvmType, nullptr, name);
    namedValues->insert({ name, alloca });
    return alloca;
}
