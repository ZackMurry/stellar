//
// Created by zack on 7/9/21.
//

#include "include/ASTFunctionInvocation.h"
#include <iostream>

using namespace std;

llvm::Value* ASTFunctionInvocation::codegen(CodegenData data) {
    llvm::Function* calleeFunc = data.module->getFunction(name);
    if (!calleeFunc) {
        cerr << "Error: unknown reference to " << name << endl;
        exit(EXIT_FAILURE);
    }
    if (calleeFunc->arg_size() != args.size() && !calleeFunc->isVarArg()) {
        cerr << "Error: incorrect number of arguments passed to function " << name << endl;
        exit(EXIT_FAILURE);
    }
    vector<llvm::Value*> argsV;
    for (auto & arg : args) {
        argsV.push_back(arg->codegen(data));
    }
    return data.builder->CreateCall(calleeFunc, argsV, "calltmp");
}
