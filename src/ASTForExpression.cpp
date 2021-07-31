//
// Created by zack on 7/25/21.
//

#include "include/ASTForExpression.h"

llvm::Value * ASTForExpression::codegen(llvm::IRBuilder<> *builder,
                                        llvm::LLVMContext *context,
                                        llvm::BasicBlock *entryBlock,
                                        std::map<std::string, llvm::Value *> *namedValues,
                                        llvm::Module *module,
                                        map<string, string> *objectTypes,
                                        map<string, ClassData> *classes) {
    if (initializer) {
        // todo: reduce scope of variable declarations to loop body instead of parent
        initializer->codegen(builder, context, entryBlock, namedValues, module, objectTypes, classes);
    }
    llvm::Value* initConditionVal = condition->codegen(builder, context, entryBlock, namedValues, module, objectTypes, classes);
    auto* loopBB = llvm::BasicBlock::Create(*context, "forbody");
    auto* mergeBB = llvm::BasicBlock::Create(*context, "mergefor");
    builder->GetInsertBlock()->getParent()->getBasicBlockList().push_back(loopBB);
    builder->CreateCondBr(initConditionVal, loopBB, mergeBB);
    builder->SetInsertPoint(loopBB);
    for (auto const& line : body) {
        line->codegen(builder, context, entryBlock, namedValues, module, objectTypes, classes);
    }
    action->codegen(builder, context, entryBlock, namedValues, module, objectTypes, classes);
    // Termination test
    auto* terminationVal = condition->codegen(builder, context, entryBlock, namedValues, module, objectTypes, classes);
    builder->CreateCondBr(terminationVal, loopBB, mergeBB);
    builder->GetInsertBlock()->getParent()->getBasicBlockList().push_back(mergeBB);
    builder->SetInsertPoint(mergeBB);
    return mergeBB;
}
