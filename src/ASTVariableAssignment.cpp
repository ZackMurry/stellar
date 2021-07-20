//
// Created by zack on 7/9/21.
//

#include "include/ASTVariableAssignment.h"
#include <iostream>

llvm::Value* ASTVariableAssignment::codegen(llvm::IRBuilder<>* builder,
                                            llvm::LLVMContext* context,
                                            llvm::BasicBlock* entryBlock,
                                            map<string, llvm::Value*>* namedValues,
                                            llvm::Module* module,
                                            map<string, string>* objectTypes,
                                            map<string, ClassData>* classes) {
    cout << "Getting named val" << endl;
    if (!namedValues->count(name)) {
        cerr << "Error: illegal use of undeclared variable '" << name << "'" << endl;
        exit(EXIT_FAILURE);
    }
    llvm::Value* var = namedValues->at(name);
    builder->CreateStore(value->codegen(builder, context, entryBlock, namedValues, module, objectTypes, classes), var);
    return var;
}
