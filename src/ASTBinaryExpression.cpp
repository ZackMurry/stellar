//
// Created by zack on 7/9/21.
//

#include "include/ASTBinaryExpression.h"
#include "iostream"

llvm::Value* ASTBinaryExpression::codegen(llvm::IRBuilder<>* builder,
                                          llvm::LLVMContext* context,
                                          llvm::BasicBlock* entryBlock,
                                          map<string, llvm::Value*>* namedValues,
                                          llvm::Module* module,
                                          map<string, string>* objectTypes,
                                          map<string, ClassData>* classes) {
    llvm::Value* l = lhs->codegen(builder, context, entryBlock, namedValues, module, objectTypes, classes);
    llvm::Value* r = rhs->codegen(builder, context, entryBlock, namedValues, module, objectTypes, classes);
    switch (op) {
        case OPERATOR_PLUS:
            if (l->getType()->isIntegerTy()) {
                return builder->CreateAdd(l, r, "addtmp");
            } else if (l->getType()->isFloatingPointTy()) {
                return builder->CreateFAdd(l, r, "addtmp");
            } else {
                cerr << "Error: unknown add type" << endl;
                exit(EXIT_FAILURE);
            }
        case OPERATOR_MINUS:
            if (l->getType()->isIntegerTy()) {
                return builder->CreateSub(l, r, "subtmp");
            } else if (l->getType()->isFloatingPointTy()) {
                return builder->CreateFSub(l, r, "subtmp");
            } else {
                cerr << "Error: unknown sub type" << endl;
                exit(EXIT_FAILURE);
            }
        case OPERATOR_TIMES:
            if (l->getType()->isIntegerTy()) {
                return builder->CreateMul(l, r, "multmp");
            } else if (l->getType()->isFloatingPointTy()) {
                return builder->CreateFMul(l, r, "multmp");
            } else {
                cerr << "Error: unknown mul type" << endl;
                exit(EXIT_FAILURE);
            }
        case OPERATOR_DIVIDE:
            if (l->getType()->isIntegerTy()) {
                return builder->CreateSDiv(l, r, "divtmp");
            } else if (l->getType()->isFloatingPointTy()) {
                return builder->CreateFDiv(l, r, "divtmp");
            } else {
                cerr << "Error: unknown div type" << endl;
                exit(EXIT_FAILURE);
            }
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
