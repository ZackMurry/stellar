//
// Created by zack on 7/9/21.
//

#include "include/ASTBinaryExpression.h"
#include "iostream"

llvm::Value* ASTBinaryExpression::codegen(llvm::IRBuilder<>* builder,
                                          llvm::LLVMContext* context,
                                          llvm::BasicBlock* entryBlock,
                                          map<string, llvm::Value*> namedValues,
                                          llvm::Module* module) {
    llvm::Value* l = lhs->codegen(builder, context, entryBlock, namedValues, module);
    llvm::Value* r = rhs->codegen(builder, context, entryBlock, namedValues, module);
    switch (op) {
        case OPERATOR_PLUS:
            return builder->CreateAdd(l, r, "addtmp");
        case OPERATOR_MINUS:
            return builder->CreateSub(l, r, "subtmp");
        case OPERATOR_TIMES:
            return builder->CreateMul(l, r, "multmp");
        case OPERATOR_DIVIDE:
            cerr << "Parser: division is not implemented" << endl;
            exit(EXIT_FAILURE);
        case OPERATOR_LT:
            if (l->getType()->isIntegerTy()) {
                return builder->CreateICmpSLT(l, r, "cmptmp");
            } else if (l->getType()->isFloatingPointTy()) {
                return builder->CreateFCmpOLT(l, r, "cmptmp");
            } else {
                cerr << "Error: unimplemented < operator" << endl;
                exit(EXIT_FAILURE);
            }
        case OPERATOR_GT:
            if (l->getType()->isIntegerTy()) {
                return builder->CreateICmpSGT(l, r, "cmptmp");
            } else if (l->getType()->isFloatingPointTy()) {
                return builder->CreateFCmpOGT(l, r, "cmptmp");
            } else {
                cerr << "Error: unimplemented > operator" << endl;
                exit(EXIT_FAILURE);
            }
        case OPERATOR_EQ:
            if (l->getType()->isIntegerTy()) {
                return builder->CreateICmpEQ(l, r, "cmptmp");
            } else if (l->getType()->isFloatingPointTy()) {
                return builder->CreateFCmpOEQ(l, r, "cmptmp");
            } else {
                cerr << "Error: unimplemented > operator" << endl;
                exit(EXIT_FAILURE);
            }
        case OPERATOR_LE:
            if (l->getType()->isIntegerTy()) {
                return builder->CreateICmpSLE(l, r, "cmptmp");
            } else if (l->getType()->isFloatingPointTy()) {
                return builder->CreateFCmpOLE(l, r, "cmptmp");
            } else {
                cerr << "Error: unimplemented > operator" << endl;
                exit(EXIT_FAILURE);
            }
        case OPERATOR_GE:
            if (l->getType()->isIntegerTy()) {
                return builder->CreateICmpSGE(l, r, "cmptmp");
            } else if (l->getType()->isFloatingPointTy()) {
                return builder->CreateFCmpOGE(l, r, "cmptmp");
            } else {
                cerr << "Error: unimplemented > operator" << endl;
                exit(EXIT_FAILURE);
            }
        case OPERATOR_NE:
            if (l->getType()->isIntegerTy()) {
                return builder->CreateICmpNE(l, r, "cmptmp");
            } else if (l->getType()->isFloatingPointTy()) {
                return builder->CreateFCmpONE(l, r, "cmptmp");
            } else {
                cerr << "Error: unimplemented > operator" << endl;
                exit(EXIT_FAILURE);
            }
        default:
            cerr << "Error: unimplemented binary expression" << endl;
            exit(EXIT_FAILURE);
    }
}
