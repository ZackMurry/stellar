//
// Created by zack on 7/9/21.
//

#include "include/ASTFunctionDefinition.h"
#include <iostream>
#include "llvm/IR/Verifier.h"

llvm::Value* ASTFunctionDefinition::codegen(CodegenData data) {
    cout << "FuncDef codegen for " << name << endl;
    llvm::Function* func = data.module->getFunction(name);
    if (func && !func->empty()) {
        cerr << "Error: function " << name << " cannot be redefined" << endl;
        exit(EXIT_FAILURE);
    }
    map<string, string> oldObjTypes;
    for (const auto& objType : *data.objectTypes) {
        oldObjTypes.insert(objType);
    }
    data.objectTypes->clear();
    vector<llvm::Type*> argTypes;
    if (!func) {
        for (const auto& arg : args) {
            cout << "Processing arg " << arg->name << " of type " << arg->type.type << endl;
            llvm::Type* llvmType;
            string genericType = convertVariableTypeToString(arg->type);
            int ivt = getPrimitiveVariableTypeFromString(genericType);
            if (ivt != -1) {
                llvmType = getLLVMTypeByPrimitiveVariableType((PrimitiveVariableType) ivt, data.context);
            } else if (data.classes->count(genericType)) {
                cout << "Arg " << arg->name << " is an object" << endl;
                llvmType = llvm::PointerType::getUnqual(data.classes->at(genericType).type);
                data.objectTypes->insert({ arg->name, genericType });
            } else {
                cerr << "Error: unknown type " << genericType << endl;
                exit(EXIT_FAILURE);
            }
            argTypes.push_back(llvmType);
        }
        cout << "Args processed" << endl;
        llvm::Type* llvmReturnType;
        auto genericReturnType = convertVariableTypeToString(returnType);
        int rtt = getPrimitiveVariableTypeFromString(genericReturnType);
        if (rtt != -1) {
            llvmReturnType = getLLVMTypeByPrimitiveVariableType((PrimitiveVariableType) rtt, data.context);
        } else if (data.classes->count(genericReturnType)) {
            cout << "Function returns object" << endl;
            // https://mapping-high-level-constructs-to-llvm-ir.readthedocs.io/en/latest/basic-constructs/functions.html
            // This says that you need to use the return value as a parameter to the function to implement returning objects,
            // but returning a pointer seems to work fine. They might have been only talking about pass-by-value
            llvmReturnType = llvm::PointerType::get(data.classes->at(genericReturnType).type, 0);
        } else {
            cerr << "Error: unknown return type " << genericReturnType << endl;
            exit(EXIT_FAILURE);
        }
        llvm::FunctionType* ft = llvm::FunctionType::get(llvmReturnType, argTypes, false);
        func = llvm::Function::Create(ft, llvm::Function::ExternalLinkage, name, *data.module);
        unsigned index = 0;
        for (auto &arg : func->args()) {
            arg.setName(args[index++]->name);
        }
    } else {
        // Get arg types
        for (const auto& arg : func->args()) {
            argTypes.push_back(arg.getType());
        }
    }
    llvm::BasicBlock* bb = llvm::BasicBlock::Create(*data.context, "entry", func);
    cout << "Created basic block" << endl;
    data.builder->SetInsertPoint(bb);
    auto *oldNamedValues = new map<string, llvm::Value*>();
    for (const auto& namedVal : *data.namedValues) {
        oldNamedValues->insert(namedVal);
    }
    data.namedValues->clear();
    unsigned index = 0;
    for (auto &arg : func->args()) {
        llvm::IRBuilder<> tempBuilder(&func->getEntryBlock(), func->getEntryBlock().begin());
        llvm::AllocaInst* alloca = tempBuilder.CreateAlloca(argTypes[index++], nullptr, arg.getName());
        data.builder->CreateStore(&arg, alloca);
        data.namedValues->insert({ arg.getName().str(), alloca });
        if (arg.getType()->isPointerTy() && arg.getType()->getPointerElementType()->isStructTy()) {
            data.objectTypes->insert({ arg.getName().str(), convertVariableTypeToString(args.at(index - 1)->type) });
        }
    }
    cout << "Args initialized" << endl;
    for (auto &node : body) {
        node->codegen(data);
    }
    cout << "Function codegen done" << endl;
    // If the final block is empty, add a return statement to it so that it is not empty
//    if (data.builder->GetInsertBlock()->empty() || !data.builder->GetInsertBlock()->end()->isTerminator()) {
        data.builder->CreateRetVoid();
//    }
    data.builder->SetInsertPoint(&data.entryBlock->getParent()->back());
    cout << "Verifying function " << name << endl;
    llvm::verifyFunction(*func);
    // Restore named values
    data.namedValues->clear();
    data.objectTypes->clear();
    for (const auto& oldNamedVal : *oldNamedValues) {
        data.namedValues->insert(oldNamedVal);
    }
    for (const auto& oldObjType : oldObjTypes) {
        data.objectTypes->insert(oldObjType);
    }
    return func;
}
