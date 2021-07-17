//
// Created by zack on 7/17/21.
//

#include "include/ASTMethodCall.h"

llvm::Value * ASTMethodCall::codegen(llvm::IRBuilder<> *builder,
                                     llvm::LLVMContext *context,
                                     llvm::BasicBlock *entryBlock,
                                     std::map<std::string, llvm::Value *> *namedValues,
                                     llvm::Module *module,
                                     map<string, string> *objectTypes,
                                     map<string, ClassData> *classes) {
    string name = objectTypes->at(identifier) + "__" + methodName;
    llvm::Function* calleeFunc = module->getFunction(name);
    if (!calleeFunc) {
        cerr << "Error: unknown reference to " << name << endl;
        exit(EXIT_FAILURE);
    }
    if (calleeFunc->arg_size() != args.size() + 1) {
        cerr << "Error: incorrect number of arguments passed to method " << methodName << " of object " << identifier << endl;
        exit(EXIT_FAILURE);
    }
    vector<llvm::Value*> argsV;
    for (auto & arg : args) {
        argsV.push_back(arg->codegen(builder, context, entryBlock, namedValues, module, objectTypes, classes));
    }
    argsV.push_back(builder->CreateLoad(namedValues->at(identifier)));
    return builder->CreateCall(calleeFunc, argsV, "calltmp");
}
