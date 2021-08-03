//
// Created by zack on 7/25/21.
//

#include "include/ASTForExpression.h"

llvm::Value* ASTForExpression::codegen(CodegenData data) {
    if (initializer) {
        // todo: reduce scope of variable declarations to loop body instead of parent
        initializer->codegen(data);
    }
    auto* loopBB = llvm::BasicBlock::Create(*data.context, "forbody");
    auto* mergeBB = llvm::BasicBlock::Create(*data.context, "mergefor");
    if (condition) {
        llvm::Value* initConditionVal = condition->codegen(data);
        data.builder->CreateCondBr(initConditionVal, loopBB, mergeBB);
    } else {
        data.builder->CreateBr(loopBB);
    }
    data.builder->GetInsertBlock()->getParent()->getBasicBlockList().push_back(loopBB);
    data.builder->SetInsertPoint(loopBB);
    for (auto const& line : body) {
        line->codegen(data);
    }
    if (action) {
        action->codegen(data);
    }
    // Termination test
    if (condition) {
        auto* terminationVal = condition->codegen(data);
        data.builder->CreateCondBr(terminationVal, loopBB, mergeBB);
    } else {
        data.builder->CreateBr(loopBB);
    }
    data.builder->GetInsertBlock()->getParent()->getBasicBlockList().push_back(mergeBB);
    data.builder->SetInsertPoint(mergeBB);
    return mergeBB;
}
