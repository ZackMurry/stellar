//
// Created by zack on 7/15/21.
//

#include "include/ASTClassInstantiation.h"
#include "include/ASTClassDefinition.h"
#include "include/ASTFunctionInvocation.h"
#include "include/ASTStringExpression.h"

llvm::Value* ASTClassInstantiation::codegen(CodegenData data) {
    cout << "ASTClassInstantiation::codegen" << endl;
    string genericClassName = className;
    if (!genericTypes.empty()) {
        genericClassName += "<";
        for (int i = 0; i < genericTypes.size(); i++) {
            if (i != 0) {
                genericClassName += ",";
            }
            if (data.generics->count(genericTypes.at(i).type)) {
                genericClassName += convertVariableTypeToString(data.generics->at(genericTypes.at(i).type));
            } else {
                genericClassName += convertVariableTypeToString(genericTypes.at(i));
            }
        }
        genericClassName += ">";
    }
    if (!data.classes->count(genericClassName)) {
        cerr << "Error: expected valid class type for class instantiation but instead found " << genericClassName << endl;
        exit(EXIT_FAILURE);
    }
    auto classData = data.classes->at(genericClassName);
    if (classData.isAbstract) {
        cerr << "Error: illegal instantiation of abstract class " << genericClassName << endl;
        exit(EXIT_FAILURE);
    }
    auto alloca = data.builder->CreateAlloca(llvm::PointerType::getUnqual(classData.type), nullptr, identifier);
    auto inst = llvm::CallInst::CreateMalloc(
            data.builder->GetInsertBlock(),
            llvm::Type::getInt64PtrTy(*data.context),
            classData.type,
            llvm::ConstantExpr::getSizeOf(classData.type),
            nullptr,
            nullptr,
            identifier);
    data.builder->CreateStore(data.builder->Insert(inst), alloca);
    data.namedValues->insert({ identifier, alloca });
    data.objectTypes->insert({ identifier, genericClassName });
    // todo: constructors that have no parameters but still have effects
    if (!args.empty()) {
        llvm::Function* constructor = data.module->getFunction(genericClassName + "__new");
        if (!constructor) {
            cerr << "Error: no constructor found for class " << genericClassName << endl;
            exit(EXIT_FAILURE);
        }
        if (constructor->arg_size() != args.size() + 1) {
            cerr << "Error: incorrect number of arguments passed to constructor of " << genericClassName << endl;
            exit(EXIT_FAILURE);
        }
        vector<llvm::Value*> argsV;
        for (auto & arg : args) {
            argsV.push_back(arg->codegen(data));
        }
        argsV.push_back(data.builder->CreateLoad(alloca)); // Add object instance parameter
        data.builder->CreateCall(constructor, argsV, "newtmp");
    }
    if (classData.vtableGlobal) {
        vector<llvm::Value*> elementIndex = { llvm::ConstantInt::get(*data.context, llvm::APInt(32, 0)), llvm::ConstantInt::get(*data.context, llvm::APInt(32, 0)) };
        auto gep = data.builder->CreateGEP(data.builder->CreateLoad(alloca), elementIndex);
        data.builder->CreateStore(classData.vtableGlobal, gep);
    }
    return alloca;
}
