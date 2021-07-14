//
// Created by zack on 7/9/21.
//

#include "include/ASTVariableExpression.h"
#include <iostream>

llvm::Value* ASTVariableExpression::codegen(llvm::IRBuilder<>* builder,
                                            llvm::LLVMContext* context,
                                            llvm::BasicBlock* entryBlock,
                                            map<string, llvm::Value*>* namedValues,
                                            llvm::Module* module) {
    llvm::Value* v = namedValues->at(name);
    if (!v) {
        cerr << "Parser: undeclared variable " << name << endl;
        exit(EXIT_FAILURE);
    }
    return builder->CreateLoad(v, name.c_str());
}
