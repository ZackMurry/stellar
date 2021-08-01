//
// Created by zack on 8/1/21.
//

#include "include/ASTLogicalExpression.h"

llvm::Value* ASTLogicalExpression::codegen(llvm::IRBuilder<> *builder,
                                            llvm::LLVMContext *context,
                                            llvm::BasicBlock *entryBlock,
                                            std::map<std::string, llvm::Value *> *namedValues,
                                            llvm::Module *module,
                                            map<string, string> *objectTypes,
                                            map<string, ClassData> *classes) {
    auto lhsValue = lhs->codegen(builder, context, entryBlock, namedValues, module, objectTypes, classes);
    auto ifBB = llvm::BasicBlock::Create(*context, "condif");
    auto mergeBB = llvm::BasicBlock::Create(*context, "mergecond");
    auto result = builder->CreateAlloca(llvm::Type::getInt1Ty(*context), nullptr, "condtmp");
    builder->CreateStore(lhsValue, result);
    if (conjunction == CONJUNCTION_AND) {
        builder->CreateCondBr(lhsValue, ifBB, mergeBB);
    } else {
        // CONJUNCTION_OR
        builder->CreateCondBr(lhsValue, mergeBB, ifBB);
    }
    builder->GetInsertBlock()->getParent()->getBasicBlockList().push_back(ifBB);
    builder->SetInsertPoint(ifBB);
    auto rhsValue = rhs->codegen(builder, context, entryBlock, namedValues, module, objectTypes, classes);
    builder->CreateStore(rhsValue, result);
    builder->CreateBr(mergeBB);
    builder->GetInsertBlock()->getParent()->getBasicBlockList().push_back(mergeBB);
    builder->SetInsertPoint(mergeBB);
    return builder->CreateLoad(result);
}
