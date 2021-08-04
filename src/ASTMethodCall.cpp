//
// Created by zack on 7/17/21.
//

#include "include/ASTMethodCall.h"
#include "include/ASTClassFieldAccess.h"

llvm::Value* ASTMethodCall::codegen(CodegenData data) {
    cout << "ASTMethodCall::codegen" << endl;
    auto parent = object->codegen(data);
    if (!parent->getType()->isPointerTy() || !parent->getType()->getPointerElementType()->isStructTy()) {
        cerr << "Error: unexpected method call of non-object variable" << endl;
        exit(EXIT_FAILURE);
    }
    string className = parent->getType()->getPointerElementType()->getStructName().str();
    if (!data.classes->count(className)) {
        cerr << "Error: unknown class " << className << " of object" << endl;
        exit(EXIT_FAILURE);
    }
    string name = className + "__" + methodName;
    llvm::Function* method = data.module->getFunction(name);
    string parentClass = data.classes->at(className).parent;
    while (!method && !parentClass.empty()) {
        name = parentClass + "__" + methodName;
        method = data.module->getFunction(name);
        parentClass = data.classes->at(parentClass).parent;
    }
    if (!method) {
        cerr << "Error: unknown reference to " << name << endl;
        exit(EXIT_FAILURE);
    }
    if (method->arg_size() != args.size() + 1) {
        cerr << "Error: incorrect number of arguments passed to method " << methodName << " of class " << className << " (expected " << method->arg_size() - 1 << " but got " << args.size() << ")" << endl;
        exit(EXIT_FAILURE);
    }
    vector<llvm::Value*> argsV;
    for (auto & arg : args) {
        argsV.push_back(arg->codegen(data));
    }
    argsV.push_back(parent);
    return data.builder->CreateCall(method, argsV, "calltmp");
}
