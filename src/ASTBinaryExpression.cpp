//
// Created by zack on 7/9/21.
//

#include "include/ASTBinaryExpression.h"
#include "iostream"

llvm::Value* ASTBinaryExpression::codegen(CodegenData data) {
    if (op == OPERATOR_AND || op == OPERATOR_OR) {
        auto lhsValue = lhs->codegen(data);
        auto ifBB = llvm::BasicBlock::Create(*data.context, "condif");
        auto mergeBB = llvm::BasicBlock::Create(*data.context, "mergecond");
        auto result = data.builder->CreateAlloca(llvm::Type::getInt1Ty(*data.context), nullptr, "condtmp");
        data.builder->CreateStore(lhsValue, result);
        if (op == OPERATOR_AND) {
            data.builder->CreateCondBr(lhsValue, ifBB, mergeBB);
        } else {
            data.builder->CreateCondBr(lhsValue, mergeBB, ifBB);
        }
        data.builder->GetInsertBlock()->getParent()->getBasicBlockList().push_back(ifBB);
        data.builder->SetInsertPoint(ifBB);
        auto rhsValue = rhs->codegen(data);
        data.builder->CreateStore(rhsValue, result);
        data.builder->CreateBr(mergeBB);
        data.builder->GetInsertBlock()->getParent()->getBasicBlockList().push_back(mergeBB);
        data.builder->SetInsertPoint(mergeBB);
        return data.builder->CreateLoad(result);
    }
    llvm::Value* l = lhs->codegen(data);
    llvm::Value* r = rhs->codegen(data);
    switch (op) {
        case OPERATOR_PLUS:
            if (l->getType()->isIntegerTy()) {
                return data.builder->CreateAdd(l, r, "addtmp");
            } else if (l->getType()->isFloatingPointTy()) {
                return data.builder->CreateFAdd(l, r, "addtmp");
            } else {
                cerr << "Error: unknown add type" << endl;
                exit(EXIT_FAILURE);
            }
        case OPERATOR_MINUS:
            if (l->getType()->isIntegerTy()) {
                return data.builder->CreateSub(l, r, "subtmp");
            } else if (l->getType()->isFloatingPointTy()) {
                return data.builder->CreateFSub(l, r, "subtmp");
            } else {
                cerr << "Error: unknown sub type" << endl;
                exit(EXIT_FAILURE);
            }
        case OPERATOR_TIMES:
            if (l->getType()->isIntegerTy()) {
                return data.builder->CreateMul(l, r, "multmp");
            } else if (l->getType()->isFloatingPointTy()) {
                return data.builder->CreateFMul(l, r, "multmp");
            } else {
                cerr << "Error: unknown mul type" << endl;
                exit(EXIT_FAILURE);
            }
        case OPERATOR_DIVIDE:
            if (l->getType()->isIntegerTy()) {
                return data.builder->CreateSDiv(l, r, "divtmp");
            } else if (l->getType()->isFloatingPointTy()) {
                return data.builder->CreateFDiv(l, r, "divtmp");
            } else {
                cerr << "Error: unknown div type" << endl;
                exit(EXIT_FAILURE);
            }
        case OPERATOR_LT:
            if (l->getType()->isIntegerTy()) {
                return data.builder->CreateICmpSLT(l, r, "cmptmp");
            } else if (l->getType()->isFloatingPointTy()) {
                return data.builder->CreateFCmpOLT(l, r, "cmptmp");
            } else {
                cerr << "Error: unimplemented < operator" << endl;
                exit(EXIT_FAILURE);
            }
        case OPERATOR_GT:
            if (l->getType()->isIntegerTy()) {
                return data.builder->CreateICmpSGT(l, r, "cmptmp");
            } else if (l->getType()->isFloatingPointTy()) {
                return data.builder->CreateFCmpOGT(l, r, "cmptmp");
            } else {
                cerr << "Error: unimplemented > operator" << endl;
                exit(EXIT_FAILURE);
            }
        case OPERATOR_EQ:
            if (l->getType()->isIntegerTy()) {
                return data.builder->CreateICmpEQ(l, r, "cmptmp");
            } else if (l->getType()->isFloatingPointTy()) {
                return data.builder->CreateFCmpOEQ(l, r, "cmptmp");
            } else {
                cerr << "Error: unimplemented == operator" << endl;
                exit(EXIT_FAILURE);
            }
        case OPERATOR_LE:
            if (l->getType()->isIntegerTy()) {
                return data.builder->CreateICmpSLE(l, r, "cmptmp");
            } else if (l->getType()->isFloatingPointTy()) {
                return data.builder->CreateFCmpOLE(l, r, "cmptmp");
            } else {
                cerr << "Error: unimplemented <= operator" << endl;
                exit(EXIT_FAILURE);
            }
        case OPERATOR_GE:
            if (l->getType()->isIntegerTy()) {
                return data.builder->CreateICmpSGE(l, r, "cmptmp");
            } else if (l->getType()->isFloatingPointTy()) {
                return data.builder->CreateFCmpOGE(l, r, "cmptmp");
            } else {
                cerr << "Error: unimplemented >= operator" << endl;
                exit(EXIT_FAILURE);
            }
        case OPERATOR_NE:
            if (l->getType()->isIntegerTy()) {
                return data.builder->CreateICmpNE(l, r, "cmptmp");
            } else if (l->getType()->isFloatingPointTy()) {
                return data.builder->CreateFCmpONE(l, r, "cmptmp");
            } else {
                cerr << "Error: unimplemented != operator" << endl;
                exit(EXIT_FAILURE);
            }
        case OPERATOR_MODULO:
            if (l->getType()->isIntegerTy()) {
                return data.builder->CreateSRem(l, r, "modtmp");
            } else if (l->getType()->isFloatingPointTy()) {
                return data.builder->CreateFRem(l, r, "modtmp");
            } else {
                cerr << "Error: unimplemented % operator" << endl;
                exit(EXIT_FAILURE);
            }
        default:
            cerr << "Error: unimplemented binary expression" << endl;
            exit(EXIT_FAILURE);
    }
}
