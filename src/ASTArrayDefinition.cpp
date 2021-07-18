//
// Created by zack on 7/9/21.
//

#include "include/ASTArrayDefinition.h"

llvm::Value* ASTArrayDefinition::codegen(llvm::IRBuilder<>* builder,
                                         llvm::LLVMContext* context,
                                         llvm::BasicBlock* entryBlock,
                                         map<string, llvm::Value*>* namedValues,
                                         llvm::Module* module,
                                         map<string, string>* objectTypes,
                                         map<string, ClassData>* classes) {
    llvm::Type* llvmElType = getLLVMTypeByVariableType(elementType, context);
    auto alloca = builder->CreateAlloca(llvmElType, nullptr, "new_arr");
    auto inst = llvm::CallInst::CreateMalloc(
            builder->GetInsertBlock(),
            llvm::Type::getInt64PtrTy(*context),
            llvmElType,
            llvm::ConstantExpr::getSizeOf(llvmElType),
            length->codegen(builder, context, entryBlock, namedValues, module, objectTypes, classes),
            nullptr,
            "arr_malloc");
    builder->CreateStore(builder->Insert(inst), alloca);
    namedValues->insert({ name, inst });
    return inst;
}

