//
// Created by zack on 7/9/21.
//

#include "include/ASTVariableExpression.h"
#include <iostream>

llvm::Value* ASTVariableExpression::codegen(llvm::IRBuilder<>* builder,
                                            llvm::LLVMContext* context,
                                            llvm::BasicBlock* entryBlock,
                                            map<string, llvm::Value*>* namedValues,
                                            llvm::Module* module,
                                            map<string, string>* objectTypes,
                                            map<string, ClassData>* classes) {
    if (!namedValues->count(name)) {
        cerr << "Error: undeclared variable " << name << endl;
        exit(EXIT_FAILURE);
    }
    llvm::Value* v = namedValues->at(name);
    return builder->CreateLoad(v, name.c_str());
}
