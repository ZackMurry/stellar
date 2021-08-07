//
// Created by zack on 7/18/21.
//

#include "include/ASTNewExpression.h"

llvm::Value* ASTNewExpression::codegen(CodegenData data) {
    string genericClassName = convertVariableTypeToString(classType);
    if (!data.classes->count(genericClassName)) {
        cerr << "Error: expected valid class type for class instantiation but instead found " << genericClassName << endl;
        exit(EXIT_FAILURE);
    }
    auto cd = data.classes->at(genericClassName);
    auto alloca = data.builder->CreateAlloca(llvm::PointerType::getUnqual(cd.type), nullptr, "new_exp");
    auto inst = llvm::CallInst::CreateMalloc(
            data.builder->GetInsertBlock(),
            llvm::Type::getInt64PtrTy(*data.context),
            cd.type,
            llvm::ConstantExpr::getSizeOf(cd.type),
            nullptr,
            nullptr);
    data.builder->CreateStore(data.builder->Insert(inst), alloca);
    auto load = data.builder->CreateLoad(alloca);
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
        argsV.push_back(load); // Add object instance parameter
        data.builder->CreateCall(constructor, argsV, "newtmp");
    }
    if (cd.vtableGlobal) {
        vector<llvm::Value*> elementIndex = { llvm::ConstantInt::get(*data.context, llvm::APInt(32, 0)), llvm::ConstantInt::get(*data.context, llvm::APInt(32, 0)) };
        auto gep = data.builder->CreateGEP(data.builder->CreateLoad(alloca), elementIndex);
        data.builder->CreateStore(cd.vtableGlobal, gep);
    }
    return load;
}
