//
// Created by zack on 7/9/21.
//

#include "include/ASTFunctionInvocation.h"
#include <iostream>

using namespace std;

llvm::Value* ASTFunctionInvocation::codegen(llvm::IRBuilder<>* builder,
                                            llvm::LLVMContext* context,
                                            llvm::BasicBlock* entryBlock,
                                            map<string, llvm::Value*>* namedValues,
                                            llvm::Module* module,
                                            map<string, string>* objectTypes,
                                            map<string, ClassData>* classes) {
    if (name == "__init_arr") {
        cout << "__init_arr :)" << endl;
        auto targetArr = args[0]->codegen(builder, context, entryBlock, namedValues, module, objectTypes, classes);
        auto inst = llvm::CallInst::CreateMalloc(
                builder->GetInsertBlock(),
                llvm::Type::getInt64PtrTy(*context),
                targetArr->getType()->getPointerElementType(),
                llvm::ConstantExpr::getSizeOf(targetArr->getType()->getPointerElementType()),
                nullptr,
                nullptr,
                "arr_malloc");
        return builder->CreateStore(builder->Insert(inst), targetArr);
    }
    llvm::Function* calleeFunc = module->getFunction(name);
    if (!calleeFunc) {
        cerr << "Error: unknown reference to " << name << endl;
        exit(EXIT_FAILURE);
    }
    if (calleeFunc->arg_size() != args.size()) {
        cerr << "Error: incorrect number of arguments passed to function " << name << endl;
        exit(EXIT_FAILURE);
    }
    vector<llvm::Value*> argsV;
    for (auto & arg : args) {
        argsV.push_back(arg->codegen(builder, context, entryBlock, namedValues, module, objectTypes, classes));
    }
    return builder->CreateCall(calleeFunc, argsV, "calltmp");
}
