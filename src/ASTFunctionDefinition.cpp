//
// Created by zack on 7/9/21.
//

#include "include/ASTFunctionDefinition.h"
#include <iostream>
#include "llvm/IR/Verifier.h"

llvm::Value* ASTFunctionDefinition::codegen(llvm::IRBuilder<>* builder,
                                            llvm::LLVMContext* context,
                                            llvm::BasicBlock* entryBlock,
                                            map<string, llvm::Value*>* namedValues,
                                            llvm::Module* module,
                                            map<string, string>* objectTypes,
                                            map<string, ClassData>* classes) {
    cout << "FuncDef codegen" << endl;
    llvm::Function* func = module->getFunction(name);
    if (func && !func->empty()) {
        cerr << "Error: function " << name << " cannot be redefined" << endl;
        exit(EXIT_FAILURE);
    }
    map<string, string> oldObjTypes;
    for (const auto& objType : *objectTypes) {
        oldObjTypes.insert(objType);
    }
    objectTypes->clear();
    vector<llvm::Type*> argTypes;
    if (!func) {
        for (const auto& arg : args) {
            cout << "Processing arg " << arg->getName() << " of type " << arg->getType() << endl;
            llvm::Type* llvmType;
            int ivt = getVariableTypeFromString(arg->getType());
            if (ivt != -1) {
                llvmType = getLLVMTypeByVariableType((VariableType) ivt, context);
            } else if (classes->count(arg->getType())) {
                cout << "Arg " << arg->getName() << " is an object" << endl;
                llvmType = llvm::PointerType::get(classes->at(arg->getType()).type, 0);
                objectTypes->insert({ arg->getName(), arg->getType() });
            } else {
                cerr << "Error: unknown type " << arg->getType() << endl;
                exit(EXIT_FAILURE);
            }
            argTypes.push_back(llvmType);
        }
        cout << "Args processed" << endl;
        llvm::Type* llvmReturnType;
        int rtt = getVariableTypeFromString(returnType);
        if (rtt != -1) {
            llvmReturnType = getLLVMTypeByVariableType((VariableType) rtt, context);
        } else if (classes->count(returnType)) {
            cout << "Function returns object" << endl;
            // https://mapping-high-level-constructs-to-llvm-ir.readthedocs.io/en/latest/basic-constructs/functions.html
            // This says that you need to use the return value as a parameter to the function to implement returning objects,
            // but returning a pointer seems to work fine. They might have been only talking about pass-by-value
            llvmReturnType = llvm::PointerType::get(classes->at(returnType).type, 0);
        } else {
            cerr << "Error: unknown return type " << returnType << endl;
            exit(EXIT_FAILURE);
        }
        llvm::FunctionType* ft = llvm::FunctionType::get(llvmReturnType, argTypes, false);
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
    auto *oldNamedValues = new map<string, llvm::Value*>();
    for (const auto& namedVal : *namedValues) {
        oldNamedValues->insert(namedVal);
    }
    namedValues->clear();
    unsigned index = 0;
    for (auto &arg : func->args()) {
        llvm::IRBuilder<> tempBuilder(&func->getEntryBlock(), func->getEntryBlock().begin());
        llvm::AllocaInst* alloca = tempBuilder.CreateAlloca(argTypes[index++], nullptr, arg.getName());
        builder->CreateStore(&arg, alloca);
        namedValues->insert({ arg.getName().str(), alloca });
    }
    cout << "Args initialized" << endl;
    for (auto &node : body) {
        node->codegen(builder, context, entryBlock, namedValues, module, objectTypes, classes);
    }
    cout << "Function codegen done" << endl;
    // If the final block is empty, add a return statement to it so that it is not empty
    if (builder->GetInsertBlock()->empty() || !builder->GetInsertBlock()->end()->isTerminator()) {
        builder->CreateRetVoid();
    }
    builder->SetInsertPoint(entryBlock);
    cout << "Verifying function " << name << endl;
    llvm::verifyFunction(*func);
    // Restore named values
    namedValues->clear();
    objectTypes->clear();
    for (const auto& oldNamedVal : *oldNamedValues) {
        namedValues->insert(oldNamedVal);
    }
    for (const auto& oldObjType : oldObjTypes) {
        objectTypes->insert(oldObjType);
    }
    return func;
}
