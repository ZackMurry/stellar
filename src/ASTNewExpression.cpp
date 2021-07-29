//
// Created by zack on 7/18/21.
//

#include "include/ASTNewExpression.h"

llvm::Value * ASTNewExpression::codegen(llvm::IRBuilder<> *builder,
                                        llvm::LLVMContext *context,
                                        llvm::BasicBlock *entryBlock,
                                        map<string, llvm::Value *> *namedValues,
                                        llvm::Module *module, map<string, string> *objectTypes,
                                        map<string, ClassData> *classes) {

    if (!classes->count(className)) {
        cerr << "Error: expected valid class type for class instantiation but instead found " << className << endl;
        exit(EXIT_FAILURE);
    }
    auto c = classes->at(className).type;
    auto alloca = builder->CreateAlloca(llvm::PointerType::getUnqual(c), nullptr, "new_exp");
    auto inst = llvm::CallInst::CreateMalloc(
            builder->GetInsertBlock(),
            llvm::Type::getInt64PtrTy(*context),
            c,
            llvm::ConstantExpr::getSizeOf(c),
            nullptr,
            nullptr);
    builder->CreateStore(builder->Insert(inst), alloca);
    // todo: constructors that have no parameters but still have effects
    auto load = builder->CreateLoad(alloca);
    if (!args.empty()) {
        llvm::Function* constructor = module->getFunction(className + "__new");
        if (!constructor) {
            cerr << "Error: no constructor found for class " << className << endl;
            exit(EXIT_FAILURE);
        }
        if (constructor->arg_size() != args.size() + 1) {
            cerr << "Error: incorrect number of arguments passed to constructor of " << className << endl;
            exit(EXIT_FAILURE);
        }
        vector<llvm::Value*> argsV;
        for (auto & arg : args) {
            argsV.push_back(arg->codegen(builder, context, entryBlock, namedValues, module, objectTypes, classes));
        }
        argsV.push_back(load); // Add object instance parameter
        builder->CreateCall(constructor, argsV, "newtmp");
    }
    return load;
}
