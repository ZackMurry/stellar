//
// Created by zack on 7/17/21.
//

#include "include/ASTMethodCall.h"
#include "include/ASTClassFieldAccess.h"
#include "include/ASTStringExpression.h"
#include "include/ASTFunctionInvocation.h"

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
    auto classData = data.classes->at(className);

    llvm::Value* method;
    cout << "Determining if " << methodName << " of " << className << " is virtual" << endl;
    if (classData.methodAttributes.at(methodName).isVirtual) {
        int methodIndex = -1;
        bool found = false;
        cout << methodName << " is virtual" << endl;
        for (int i = 0 ; i < classData.methodOrder.size(); i++) {
            if (classData.methodOrder.at(i) == methodName) {
                methodIndex++;
                found = true;
                break;
            } else if (classData.methodAttributes.at(classData.methodOrder.at(i)).isVirtual) {
                methodIndex++;
            }
        }
        if (!found) {
            cerr << "Error: unknown method of " << className << ": " << methodName << endl;
            exit(EXIT_FAILURE);
        }
        cout << "Method index " << methodIndex << endl;
        auto vtable = data.builder->CreateLoad(data.builder->CreateStructGEP(parent->getType()->getPointerElementType(), parent, 0));
        auto gep = data.builder->CreateStructGEP(vtable->getType()->getPointerElementType(), vtable, methodIndex);
        method = data.builder->CreateLoad(gep);
        cout << "Has method " << methodName << " " << classData.methods.count(methodName) << endl;
    } else {
        cout << methodName << " is not virtual" << endl;
        method = classData.methods.at(methodName);
        if (!method) {
            cerr << "Error: unknown method of " << className << ": " << methodName << endl;
            exit(EXIT_FAILURE);
        }
    }
    if (classData.methods.at(methodName)->arg_size() != args.size() + 1) {
        cerr << "Error: incorrect number of arguments passed to method " << methodName << " of class " << className << " (expected " << classData.methods.at(methodName)->arg_size() - 1 << " but got " << args.size() << ")" << endl;
        exit(EXIT_FAILURE);
    }
    vector<llvm::Value*> argsV;
    for (auto & arg : args) {
        argsV.push_back(arg->codegen(data));
    }
    argsV.push_back(parent);
//    vector<llvm::Value*> printfArgs;
//    printfArgs.push_back(ASTStringExpression("Method: %d\n").codegen(data));
//    printfArgs.push_back(method);
//    data.builder->CreateCall(data.module->getFunction("printf"), llvm::ArrayRef<llvm::Value*>(printfArgs));
    auto call = data.builder->CreateCall(llvm::FunctionCallee(classData.methods.at(methodName)->getFunctionType(), method), argsV, "calltmp");
    return call;
}
