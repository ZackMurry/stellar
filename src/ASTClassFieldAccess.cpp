//
// Created by zack on 7/15/21.
//

#include "include/ASTClassFieldAccess.h"

llvm::Value* ASTClassFieldAccess::codegen(llvm::IRBuilder<> *builder,
                                          llvm::LLVMContext *context,
                                          llvm::BasicBlock *entryBlock,
                                          map<string, llvm::Value *> *namedValues,
                                          llvm::Module *module,
                                          map<string, string>* objectTypes,
                                          map<string, ClassData>* classes) {
    cout << "ASTClassFieldAccess::codegen" << endl;
    auto parent = object->codegen(builder, context, entryBlock, namedValues, module, objectTypes, classes);
    if (!parent->getType()->isPointerTy() || !parent->getType()->getPointerElementType()->isStructTy()) {
        cerr << "Error: unexpected field access of non-object variable" << endl;
        exit(EXIT_FAILURE);
    }
    string className = parent->getType()->getPointerElementType()->getStructName().str();
    int fieldNumber = -1;
    int i = 0;
    for (const auto& f : classes->at(className).fields) {
        if (f.name == fieldName) {
            fieldNumber = i;
            break;
        }
        i++;
    }
    if (fieldNumber == -1) {
        cerr << "Error: unknown field of class " << className << ": " << fieldName << endl;
        exit(EXIT_FAILURE);
    }
    vector<llvm::Value*> elementIndex = { llvm::ConstantInt::get(*context, llvm::APInt(32, 0)), llvm::ConstantInt::get(*context, llvm::APInt(32, fieldNumber)) };
    llvm::Value* gep = builder->CreateGEP(parent, elementIndex);
    return builder->CreateLoad(gep);
}
