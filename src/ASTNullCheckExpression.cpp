//
// Created by zack on 7/29/21.
//

#include "include/ASTNullCheckExpression.h"

llvm::Value * ASTNullCheckExpression::codegen(llvm::IRBuilder<> *builder,
                                              llvm::LLVMContext *context,
                                              llvm::BasicBlock *entryBlock,
                                              std::map<std::string, llvm::Value *> *namedValues,
                                              llvm::Module *module,
                                              map<string, string> *objectTypes,
                                              map<string, ClassData> *classes) {
    if (!namedValues->count(identifier)) {
        cerr << "Error: unknown identifier " << identifier << endl;
        exit(EXIT_FAILURE);
    }
    if (!objectTypes->count(identifier)) {
        cerr << "Error: invalid attempt to perform null check on non-object. Null checks are only available for objects" << endl;
        exit(EXIT_FAILURE);
    }
    cout << "Parent: " << builder->GetInsertBlock()->getParent()->getName().str() << endl;
    return builder->CreateIsNull(builder->CreateLoad(namedValues->at(identifier)), "nullchktmp");
}
