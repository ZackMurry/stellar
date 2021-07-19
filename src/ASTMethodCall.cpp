//
// Created by zack on 7/17/21.
//

#include "include/ASTMethodCall.h"
#include "include/ASTClassFieldAccess.h"

llvm::Value * ASTMethodCall::codegen(llvm::IRBuilder<> *builder,
                                     llvm::LLVMContext *context,
                                     llvm::BasicBlock *entryBlock,
                                     std::map<std::string, llvm::Value *> *namedValues,
                                     llvm::Module *module,
                                     map<string, string> *objectTypes,
                                     map<string, ClassData> *classes) {
    llvm::Value* obj;
    if (identifiers.size() > 1) {
        vector<string> allButFirstIdentifier;
        for (int i = 1; i < identifiers.size(); i++) {
            allButFirstIdentifier.push_back(identifiers.at(i));
        }
        auto cfa = new ASTClassFieldAccess(identifiers.at(0), allButFirstIdentifier);
        obj = cfa->codegen(builder, context, entryBlock, namedValues, module, objectTypes, classes);
    } else {
        obj = builder->CreateLoad(namedValues->at(identifiers.at(0)));
    }
    string lastIdentifier = identifiers.at(identifiers.size() - 1);
    ClassData classData = classes->at(objectTypes->at(identifiers.at(0)));
    string className = objectTypes->at(identifiers.at(0));
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
        className = field.classType;
    }
    string name = className + "__" + methodName;
    llvm::Function* calleeFunc = module->getFunction(name);
    if (!calleeFunc) {
        cerr << "Error: unknown reference to " << name << endl;
        exit(EXIT_FAILURE);
    }
    if (calleeFunc->arg_size() != args.size() + 1) {
        cerr << "Error: incorrect number of arguments passed to method " << methodName << " of object " << lastIdentifier << endl;
        exit(EXIT_FAILURE);
    }
    vector<llvm::Value*> argsV;
    for (auto & arg : args) {
        argsV.push_back(arg->codegen(builder, context, entryBlock, namedValues, module, objectTypes, classes));
    }
    argsV.push_back(obj);
    return builder->CreateCall(calleeFunc, argsV, "calltmp");
}
