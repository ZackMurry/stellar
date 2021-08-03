//
// Created by zack on 7/9/21.
//

#include "include/ASTIfStatement.h"

llvm::Value* ASTIfStatement::codegen(CodegenData data) {
    llvm::Value* conditionValue = condition->codegen(data);
    llvm::BasicBlock* ifBB = llvm::BasicBlock::Create(*data.context, "ifbody");
    llvm::BasicBlock* elseBB = llvm::BasicBlock::Create(*data.context, "elsebody");
    llvm::BasicBlock* mergeBB = llvm::BasicBlock::Create(*data.context, "mergeif");
    data.builder->CreateCondBr(conditionValue, ifBB, elseBB);
    data.builder->GetInsertBlock()->getParent()->getBasicBlockList().push_back(ifBB);
    data.builder->GetInsertBlock()->getParent()->getBasicBlockList().push_back(elseBB);
    data.builder->SetInsertPoint(ifBB);
    for (auto const& line : ifBody) {
        line->codegen(data);
    }
    data.builder->CreateBr(mergeBB);
    data.builder->SetInsertPoint(elseBB);
    for (auto const& line : elseBody) {
        line->codegen(data);
    }
    data.builder->CreateBr(mergeBB);
    // Add merge block last so that the outer-most merge block in nested if statements occurs last
    data.builder->GetInsertBlock()->getParent()->getBasicBlockList().push_back(mergeBB);
    data.builder->SetInsertPoint(mergeBB);
    return mergeBB;
}
