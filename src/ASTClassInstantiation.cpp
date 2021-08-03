//
// Created by zack on 7/15/21.
//

#include "include/ASTClassInstantiation.h"

llvm::Value* ASTClassInstantiation::codegen(llvm::IRBuilder<> *builder,
                                            llvm::LLVMContext *context,
                                            llvm::BasicBlock *entryBlock,
                                            map<string, llvm::Value *> *namedValues,
                                            llvm::Module *module,
                                            map<string, string>* objectTypes,
                                            map<string, ClassData>* classes) {
    cout << "ASTClassInstantation::codegen" << endl;
    string genericClassName = className;
    if (!genericTypes.empty()) {
        genericClassName += "<";
        for (int i = 0; i < genericTypes.size(); i++) {
            if (i != 0) {
                genericClassName += ",";
            }
            genericClassName += convertVariableTypeToString(genericTypes.at(i));
        }
        genericClassName += ">";
    }
    if (!classes->count(genericClassName)) {
        cerr << "Error: expected valid class type for class instantiation but instead found " << genericClassName << endl;
        exit(EXIT_FAILURE);
    }
    auto c = classes->at(genericClassName).type;
    auto alloca = builder->CreateAlloca(llvm::PointerType::getUnqual(c), nullptr, identifier);
    auto inst = llvm::CallInst::CreateMalloc(
            builder->GetInsertBlock(),
            llvm::Type::getInt64PtrTy(*context),
            c,
            llvm::ConstantExpr::getSizeOf(c),
            nullptr,
            nullptr,
            identifier);
    builder->CreateStore(builder->Insert(inst), alloca);
    namedValues->insert({ identifier, alloca });
    objectTypes->insert({ identifier, genericClassName });
    // todo: constructors that have no parameters but still have effects
    if (!args.empty()) {
        llvm::Function* constructor = module->getFunction(genericClassName + "__new");
        if (!constructor) {
            cerr << "Error: no constructor found for class " << genericClassName << endl;
            exit(EXIT_FAILURE);
        }
        if (constructor->arg_size() != args.size() + 1) {
            cerr << "Error: incorrect number of arguments passed to constructor of " << genericClassName << endl;
            exit(EXIT_FAILURE);
        }
        vector<llvm::Value*> argsV;
        for (auto & arg : args) {
            argsV.push_back(arg->codegen(builder, context, entryBlock, namedValues, module, objectTypes, classes));
        }
        argsV.push_back(builder->CreateLoad(alloca)); // Add object instance parameter
        builder->CreateCall(constructor, argsV, "newtmp");
    }
    return alloca;
}
