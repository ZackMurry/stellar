//
// Created by zack on 7/15/21.
//

#include "include/ASTClassFieldStore.h"
#include "include/ASTClassFieldAccess.h"

llvm::Value* ASTClassFieldStore::codegen(llvm::IRBuilder<> *builder,
                                         llvm::LLVMContext *context,
                                         llvm::BasicBlock *entryBlock,
                                         map<string, llvm::Value *> *namedValues,
                                         llvm::Module *module,
                                         map<string, string>* objectTypes,
                                         map<string, ClassData>* classes) {
    llvm::Value* obj;
    if (identifiers.size() > 1) {
        // This is basically ASTClassFieldAccess::codegen but without loading the final value :|
        vector<string> allButFirstIdentifier;
        for (int i = 1; i < identifiers.size(); i++) {
            allButFirstIdentifier.push_back(identifiers.at(i));
        }
        string identifier = identifiers.at(0);
        vector<string> fieldNames = allButFirstIdentifier;
        if (!objectTypes->count(identifier)) {
            cerr << "Error: undeclared identifier " << identifier << endl;
            exit(EXIT_FAILURE);
        }
        llvm::Value* currObj = namedValues->at(identifier);
        ClassData currObjData = classes->at(objectTypes->at(identifier));
        for (const auto& fn : fieldNames) {
            int fieldNumber = -1;
            int i = 0;
            for (const auto& f : currObjData.fields) {
                if (f.name == fn) {
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
            llvm::Value* gep = builder->CreateGEP(builder->CreateLoad(currObj), elementIndex);
            currObj = gep;
            if (!currObjData.fields.at(fieldNumber).classType.empty()) {
                currObjData = classes->at(currObjData.fields.at(fieldNumber).classType);
            } else {
                break;
            }
        }
        obj = currObj;
    } else {
        obj = namedValues->at(identifiers.at(0));
    }
    string lastIdentifier = identifiers.at(identifiers.size() - 1);
    ClassData classData = classes->at(objectTypes->at(identifiers.at(0)));
    for (int i = 1; i < identifiers.size(); i++) {
        ClassFieldType field;
        for (const auto& f : classData.fields) {
            if (f.name == identifiers.at(i)) {
                field = f;
                break;
            }
        }
        if (field.type == nullptr) {
            cerr << "Error: unknown field of object: " << identifiers.at(i) << endl;
        }
        classData = classes->at(field.classType);
    }
    int fieldNumber = -1;
    int i = 0;
    for (const auto& f : classData.fields) {
        if (f.name == fieldName) {
            fieldNumber = i;
            break;
        }
        i++;
    }
    if (fieldNumber == -1) {
        cerr << "Error: unknown field of object " << identifiers.at(identifiers.size() - 1) << ": " << fieldName << endl;
        exit(EXIT_FAILURE);
    }
    vector<llvm::Value*> elementIndex = { llvm::ConstantInt::get(*context, llvm::APInt(32, 0)), llvm::ConstantInt::get(*context, llvm::APInt(32, fieldNumber)) };
    auto load = builder->CreateLoad(obj);
    llvm::Value* gep = builder->CreateGEP(load, elementIndex);
    builder->CreateStore(value->codegen(builder, context, entryBlock, namedValues, module, objectTypes, classes), gep);
    return gep;
}
