//
// Created by zack on 7/18/21.
//

#include "include/ASTClassFieldArrayAccess.h"
#include "include/ASTArrayAccess.h"

llvm::Value* ASTClassFieldArrayAccess::codegen(llvm::IRBuilder<> *builder,
                                          llvm::LLVMContext *context,
                                          llvm::BasicBlock *entryBlock,
                                          map<string, llvm::Value *> *namedValues,
                                          llvm::Module *module,
                                          map<string, string>* objectTypes,
                                          map<string, ClassData>* classes) {
    if (!objectTypes->count(identifier)) {
        cerr << "Error: undeclared identifier " << identifier << endl;
        exit(EXIT_FAILURE);
    }
    cout << "Accessing field " << fieldName << " of " << identifier << endl;
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
    cout << "Creating GEP" << endl;
    llvm::Value* gep = builder->CreateGEP(builder->CreateLoad(namedValues->at(identifier)), elementIndex);
    auto* arrGep = builder->CreateInBoundsGEP(gep, index->codegen(builder, context, entryBlock, namedValues, module, objectTypes, classes), "acctmp");
//    arrGep->mutateType(getLLVMPtrTypeByType(gep->getType()->getPointerElementType(), context));
    return builder->CreateLoad(arrGep);
}
