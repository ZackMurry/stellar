//
// Created by zack on 7/25/21.
//

#include "include/ASTWhileExpression.h"

llvm::Value* ASTWhileExpression::codegen(CodegenData data) {
    llvm::Value* initConditionVal = condition->codegen(data);
    auto* loopBB = llvm::BasicBlock::Create(*data.context, "whilebody");
    auto* mergeBB = llvm::BasicBlock::Create(*data.context, "mergewhile");
    data.builder->GetInsertBlock()->getParent()->getBasicBlockList().push_back(loopBB);
    data.builder->CreateCondBr(initConditionVal, loopBB, mergeBB);
    data.builder->SetInsertPoint(loopBB);
    for (auto const& line : body) {
        line->codegen(data);
    }
    // Termination test
    auto* terminationVal = condition->codegen(data);
    data.builder->CreateCondBr(terminationVal, loopBB, mergeBB);
    data.builder->GetInsertBlock()->getParent()->getBasicBlockList().push_back(mergeBB);
    data.builder->SetInsertPoint(mergeBB);
    return mergeBB;
}
