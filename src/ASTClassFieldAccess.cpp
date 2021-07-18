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
    if (!objectTypes->count(identifier)) {
        cerr << "Error: undeclared identifier " << identifier << endl;
        exit(EXIT_FAILURE);
    }
    llvm::Value* currObj = namedValues->at(identifier);
    ClassData currObjData = classes->at(objectTypes->at(identifier));
    bool isFirst = true;
    for (const auto& fieldName : fieldNames) {
        int fieldNumber = -1;
        int i = 0;
        for (const auto& f : currObjData.fields) {
            if (f.name == fieldName) {
                fieldNumber = i;
                break;
            }
            i++;
        }
        if (fieldNumber == -1) {
            cerr << "Error: unknown field of object " << identifier << endl;
            exit(EXIT_FAILURE);
        }
        vector<llvm::Value*> elementIndex = { llvm::ConstantInt::get(*context, llvm::APInt(32, 0)), llvm::ConstantInt::get(*context, llvm::APInt(32, fieldNumber)) };
        cout << "Creating GEP" << endl;
        cout << "Curr obj data" << currObjData.fields.at(0).name << endl;
        llvm::Value* gep;
        if (isFirst) {
            gep = builder->CreateGEP(builder->CreateLoad(currObj), elementIndex);
            isFirst = false;
        } else {
            gep = builder->CreateGEP(currObj, elementIndex);
        }
        cout << "GEP created" << endl;
        if (gep->getType()->isPointerTy() && gep->getType()->getPointerElementType()->isPointerTy()) {
            cout << "Pointer type: " << identifier << "." << fieldName << endl;
            vector<llvm::Value*> elementIndexZero = { llvm::ConstantInt::get(*context, llvm::APInt(32, 0)), llvm::ConstantInt::get(*context, llvm::APInt(32, 0)) };
            gep = builder->CreateGEP(builder->CreateLoad(gep), elementIndexZero);
        }
        currObj = builder->CreateLoad(gep);
        cout << "Updating objData" << endl;
        if (!currObjData.fields.at(fieldNumber).classType.empty()) {
            currObjData = classes->at(currObjData.fields.at(fieldNumber).classType);
        } else {
            break;
        }
    }
    cout << "returned from class field access" << endl;
    return currObj;
}
