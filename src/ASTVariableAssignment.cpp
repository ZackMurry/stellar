//
// Created by zack on 7/9/21.
//

#include "include/ASTVariableAssignment.h"
#include <iostream>

llvm::Value* ASTVariableAssignment::codegen(llvm::IRBuilder<>* builder,
                                            llvm::LLVMContext* context,
                                            llvm::BasicBlock* entryBlock,
                                            map<string, llvm::Value*>* namedValues,
                                            llvm::Module* module) {
    cout << "Getting named val" << endl;
    cout << "named val: " << namedValues->at(name)->getName().str() << endl;
    llvm::Value* var = namedValues->at(name);
    if (!var) { // todo: this can't detect use of variables declared in a function outside of the function
        cerr << "Error: illegal use of undeclared variable '" << name << "'" << endl;
        exit(EXIT_FAILURE);
    }
    builder->CreateStore(value->codegen(builder, context, entryBlock, namedValues, module), var);
    return var;
}
