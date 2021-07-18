//
// Created by zack on 7/18/21.
//

#include "include/ASTClassFieldArrayStore.h"

llvm::Value * ASTClassFieldArrayStore::codegen(llvm::IRBuilder<> *builder,
                                               llvm::LLVMContext *context,
                                               llvm::BasicBlock *entryBlock,
                                               map<string, llvm::Value *> *namedValues,
                                               llvm::Module *module,
                                               map<string, string> *objectTypes,
                                               map<string, ClassData> *classes) {
    auto classData = classes->at(objectTypes->at(identifier));
    int fieldNumber = -1;
    int i = 0;
    for (const auto& f : classData.fields) {
        if (f.first == fieldName) {
            fieldNumber = i;
            break;
        }
        i++;
    }
    if (fieldNumber == -1) {
        cerr << "Error: unknown field of object " << identifier << ": " << fieldName << " (object is type " << objectTypes->at(identifier) << ")" << endl;
        exit(EXIT_FAILURE);
    }
    vector<llvm::Value*> elementIndex = { llvm::ConstantInt::get(*context, llvm::APInt(32, 0)), llvm::ConstantInt::get(*context, llvm::APInt(32, fieldNumber)) };
    cout << "el index: " << fieldNumber << endl;
    llvm::Value* gep = builder->CreateGEP(builder->CreateLoad(namedValues->at(identifier)), elementIndex);
    cout << "gep created" << endl;
    llvm::Value* ref = builder->CreateInBoundsGEP(gep, llvm::ArrayRef<llvm::Value*>(index->codegen(builder, context, entryBlock, namedValues, module, objectTypes, classes)), "acctmp");
    return builder->CreateStore(value->codegen(builder, context, entryBlock, namedValues, module, objectTypes, classes), ref);
}
