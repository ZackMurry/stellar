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
    cout << "ASTClassFieldStore::codegen" << endl;
    llvm::Value* obj;
    cout << to_string(identifiers.size()) << " identifiers" << endl;
    if (identifiers.size() > 1) {
        vector<string> allButFirstIdentifier;
        for (int i = 1; i < identifiers.size(); i++) {
            allButFirstIdentifier.push_back(identifiers.at(i));
        }
        auto cfa = new ASTClassFieldAccess(identifiers.at(0), allButFirstIdentifier);
        obj = cfa->codegen(builder, context, entryBlock, namedValues, module, objectTypes, classes);
    } else {
        cout << "Getting identifier" << identifiers.at(0) << endl;
        obj = namedValues->at(identifiers.at(0));
    }
    cout << "Getting classData" << endl;
    string lastIdentifier = identifiers.at(identifiers.size() - 1);
    cout << "first identifier: " << identifiers.at(0) << endl;
    cout << "first identifier type: " << objectTypes->at(identifiers.at(0)) << endl;
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
    cout << "el index: " << fieldNumber << endl;
    llvm::Value* gep;
    if (identifiers.size() == 1) {
        gep = builder->CreateGEP(builder->CreateLoad(obj), elementIndex);
    } else {
        cout << "No load" << endl;
        gep = builder->CreateGEP(obj, elementIndex);
    }
    cout << "gep created" << endl;
    return builder->CreateStore(value->codegen(builder, context, entryBlock, namedValues, module, objectTypes, classes), gep);
}
