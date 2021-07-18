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
