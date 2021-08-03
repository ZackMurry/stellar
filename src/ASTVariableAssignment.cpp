//
// Created by zack on 7/9/21.
//

#include "include/ASTVariableAssignment.h"
#include <iostream>

llvm::Value* ASTVariableAssignment::codegen(CodegenData data) {
    cout << "Getting named val" << endl;
    if (!data.namedValues->count(name)) {
        cerr << "Error: illegal use of undeclared variable '" << name << "'" << endl;
        exit(EXIT_FAILURE);
    }
    llvm::Value* var = data.namedValues->at(name);
    data.builder->CreateStore(value->codegen(data), var);
    return var;
}
