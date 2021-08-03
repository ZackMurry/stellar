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
    cout << "FuncDef codegen for " << name << endl;
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
            cout << "Processing arg " << arg->name << " of type " << arg->type.type << endl;
            llvm::Type* llvmType;
            string genericType = convertVariableTypeToString(arg->type);
            int ivt = getPrimitiveVariableTypeFromString(genericType);
            if (ivt != -1) {
                llvmType = getLLVMTypeByPrimitiveVariableType((PrimitiveVariableType) ivt, context);
            } else if (classes->count(genericType)) {
                cout << "Arg " << arg->name << " is an object" << endl;
                llvmType = llvm::PointerType::getUnqual(classes->at(genericType).type);
                objectTypes->insert({ arg->name, genericType });
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
            llvmReturnType = getLLVMTypeByPrimitiveVariableType((PrimitiveVariableType) rtt, context);
        } else if (classes->count(genericReturnType)) {
            cout << "Function returns object" << endl;
            // https://mapping-high-level-constructs-to-llvm-ir.readthedocs.io/en/latest/basic-constructs/functions.html
            // This says that you need to use the return value as a parameter to the function to implement returning objects,
            // but returning a pointer seems to work fine. They might have been only talking about pass-by-value
            llvmReturnType = llvm::PointerType::get(classes->at(genericReturnType).type, 0);
        } else {
            cerr << "Error: unknown return type " << genericReturnType << endl;
            exit(EXIT_FAILURE);
        }
        llvm::FunctionType* ft = llvm::FunctionType::get(llvmReturnType, argTypes, false);
        func = llvm::Function::Create(ft, llvm::Function::ExternalLinkage, name, *module);
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
        if (arg.getType()->isPointerTy() && arg.getType()->getPointerElementType()->isStructTy()) {
            objectTypes->insert({ arg.getName().str(), convertVariableTypeToString(args.at(index - 1)->type) });
        }
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
    builder->SetInsertPoint(&entryBlock->getParent()->back());
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
