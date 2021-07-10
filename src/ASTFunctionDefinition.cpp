//
// Created by zack on 7/9/21.
//

#include "include/ASTFunctionDefinition.h"
#include <iostream>
#include "llvm/IR/Verifier.h"

llvm::Value* ASTFunctionDefinition::codegen(llvm::IRBuilder<>* builder,
                                            llvm::LLVMContext* context,
                                            llvm::BasicBlock* entryBlock,
                                            map<string, llvm::Value*> namedValues,
                                            llvm::Module* module) {
    cout << "FuncDef codegen" << endl;
    llvm::Function* func = module->getFunction(name);
    if (func && !func->empty()) {
        cerr << "Error: function " << name << " cannot be redefined" << endl;
        exit(EXIT_FAILURE);
    }
    vector<llvm::Type*> argTypes;
    if (!func) {
        for (const auto& arg : args) {
            llvm::Type* llvmType = getLLVMTypeByVariableType(arg->getType(), context);
            if (llvmType == nullptr) {
                cerr << "Error mapping variable type to LLVM type" << endl;
                exit(EXIT_FAILURE);
            }
            argTypes.push_back(llvmType);
        }
        llvm::FunctionType* ft = llvm::FunctionType::get(getLLVMTypeByVariableType(returnType, context), argTypes, false);
        func = llvm::Function::Create(ft, llvm::Function::ExternalLinkage, name, *module);
        unsigned index = 0;
        for (auto &arg : func->args()) {
            arg.setName(args[index++]->getName());
        }
    } else {
        // Get arg types
        for (const auto& arg : func->args()) {
            argTypes.push_back(arg.getType());
        }
    }
    llvm::BasicBlock* bb = llvm::BasicBlock::Create(*context, "entry", func);
    cout << "Created basic block" << endl;
    builder->SetInsertPoint(bb);
    namedValues.clear();
    unsigned index = 0;
    for (auto &arg : func->args()) {
        llvm::IRBuilder<> tempBuilder(&func->getEntryBlock(), func->getEntryBlock().begin());
        llvm::AllocaInst* alloca = tempBuilder.CreateAlloca(argTypes[index++], nullptr, arg.getName());
        builder->CreateStore(&arg, alloca);
        namedValues[string(arg.getName())] = alloca;
    }
    for (auto &node : body) {
        node->codegen(builder, context, entryBlock, namedValues, module);
    }
    cout << "For loops done" << endl;
    // If the final block is empty, add a return statement to it so that it is not empty
    if (builder->GetInsertBlock()->empty()) {
        builder->CreateRetVoid();
    }
    builder->SetInsertPoint(entryBlock);
    llvm::verifyFunction(*func);
    return func;
}
