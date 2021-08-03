//
// Created by zack on 7/9/21.
//

#include "include/ASTVariableExpression.h"
#include <iostream>

llvm::Value* ASTVariableExpression::codegen(CodegenData data) {
    if (!data.namedValues->count(name)) {
        cerr << "Error: undeclared variable " << name << endl;
        exit(EXIT_FAILURE);
    }
    llvm::Value* v = data.namedValues->at(name);
    return data.builder->CreateLoad(v, name.c_str());
}
