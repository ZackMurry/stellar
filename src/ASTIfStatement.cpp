//
// Created by zack on 7/9/21.
//

#include "include/ASTIfStatement.h"

llvm::Value* ASTIfStatement::codegen(llvm::IRBuilder<>* builder,
                                     llvm::LLVMContext* context,
                                     llvm::BasicBlock* entryBlock,
                                     map<string, llvm::Value*>* namedValues,
                                     llvm::Module* module) {
    llvm::Value* conditionValue = condition->codegen(builder, context, entryBlock, namedValues, module);
    llvm::BasicBlock* ifBB = llvm::BasicBlock::Create(*context, "ifbody");
    llvm::BasicBlock* elseBB = llvm::BasicBlock::Create(*context, "elsebody");
    llvm::BasicBlock* mergeBB = llvm::BasicBlock::Create(*context, "mergeif");
    builder->CreateCondBr(conditionValue, ifBB, elseBB);
    builder->GetInsertBlock()->getParent()->getBasicBlockList().push_back(ifBB);
    builder->GetInsertBlock()->getParent()->getBasicBlockList().push_back(elseBB);
    builder->GetInsertBlock()->getParent()->getBasicBlockList().push_back(mergeBB);
    builder->SetInsertPoint(ifBB);
    for (auto const& line : ifBody) {
        line->codegen(builder, context, entryBlock, namedValues, module);
    }
    builder->CreateBr(mergeBB);
    builder->SetInsertPoint(elseBB);
    for (auto const& line : elseBody) {
        line->codegen(builder, context, entryBlock, namedValues, module);
    }
    builder->CreateBr(mergeBB);
    builder->SetInsertPoint(mergeBB);
    return mergeBB;
}
