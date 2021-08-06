//
// Created by zack on 7/15/21.
//

#include "include/ASTClassFieldStore.h"
#include "include/ASTClassFieldAccess.h"

llvm::Value* ASTClassFieldStore::codegen(CodegenData data) {
    cout << "ASTClassFieldStore::codegen" << endl;
    auto parent = object->codegen(data);
    if (!parent->getType()->isPointerTy() || !parent->getType()->getPointerElementType()->isStructTy()) {
        cerr << "Error: unexpected field access of non-object variable" << endl;
        exit(EXIT_FAILURE);
    }
    string className = parent->getType()->getPointerElementType()->getStructName().str();
    int fieldNumber = -1;
    int i = 0;
    for (const auto& f : data.classes->at(className).fields) {
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
    cout << "fieldNumber: " << fieldNumber << endl;
    vector<llvm::Value*> elementIndex = { llvm::ConstantInt::get(*data.context, llvm::APInt(32, 0)), llvm::ConstantInt::get(*data.context, llvm::APInt(32, fieldNumber + 1)) };
    llvm::Value* gep = data.builder->CreateGEP(parent, elementIndex);
    data.builder->CreateStore(value->codegen(data), gep);
    return gep;
}
