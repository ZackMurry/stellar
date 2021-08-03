//
// Created by zack on 7/18/21.
//

#include "include/ASTNewExpression.h"

llvm::Value* ASTNewExpression::codegen(CodegenData data) {
    if (!data.classes->count(className)) {
        cerr << "Error: expected valid class type for class instantiation but instead found " << className << endl;
        exit(EXIT_FAILURE);
    }
    auto c = data.classes->at(className).type;
    auto alloca = data.builder->CreateAlloca(llvm::PointerType::getUnqual(c), nullptr, "new_exp");
    auto inst = llvm::CallInst::CreateMalloc(
            data.builder->GetInsertBlock(),
            llvm::Type::getInt64PtrTy(*data.context),
            c,
            llvm::ConstantExpr::getSizeOf(c),
            nullptr,
            nullptr);
    data.builder->CreateStore(data.builder->Insert(inst), alloca);
    auto load = data.builder->CreateLoad(alloca);
    if (!args.empty()) {
        llvm::Function* constructor = data.module->getFunction(className + "__new");
        if (!constructor) {
            cerr << "Error: no constructor found for class " << className << endl;
            exit(EXIT_FAILURE);
        }
        if (constructor->arg_size() != args.size() + 1) {
            cerr << "Error: incorrect number of arguments passed to constructor of " << className << endl;
            exit(EXIT_FAILURE);
        }
        vector<llvm::Value*> argsV;
        for (auto & arg : args) {
            argsV.push_back(arg->codegen(data));
        }
        argsV.push_back(load); // Add object instance parameter
        data.builder->CreateCall(constructor, argsV, "newtmp");
    }
    return load;
}
