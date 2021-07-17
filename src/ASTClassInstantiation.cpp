//
// Created by zack on 7/15/21.
//

#include "include/ASTClassInstantiation.h"

llvm::Value* ASTClassInstantiation::codegen(llvm::IRBuilder<> *builder,
                                            llvm::LLVMContext *context,
                                            llvm::BasicBlock *entryBlock,
                                            map<string, llvm::Value *> *namedValues,
                                            llvm::Module *module,
                                            map<string, string>* objectTypes,
                                            map<string, ClassData>* classes) {
    if (!classes->count(className)) {
        cerr << "Error: expected valid class type for class instantiation but instead found " << className << endl;
        exit(EXIT_FAILURE);
    }
    auto c = classes->at(className).type;
    auto alloca = builder->CreateAlloca(llvm::PointerType::getUnqual(c), nullptr, identifier);
    auto inst = llvm::CallInst::CreateMalloc(
            builder->GetInsertBlock(),
            llvm::Type::getInt64PtrTy(*context),
            c,
            llvm::ConstantExpr::getSizeOf(c),
            nullptr,
            nullptr,
            identifier);
    builder->CreateStore(builder->Insert(inst), alloca);
    namedValues->insert({ identifier, alloca });
    objectTypes->insert({ identifier, className });
    return alloca;
}
