//
// Created by zack on 7/17/21.
//

#include "parser.h"
#include <string>
#ifndef STELLAR_ASTMETHODCALL_H
#define STELLAR_ASTMETHODCALL_H


class ASTMethodCall : public ASTNode {
    vector<string> identifiers;
    string methodName;
    vector<ASTNode*> args;
public:
    ASTMethodCall(vector<string> identifiers, string methodName, vector<ASTNode*> args) : identifiers(move(identifiers)), methodName(move(methodName)), args(move(args)) {}
    string toString() override {
        return "[METHOD_CALL: object: " + to_string(identifiers.size()) + " method: " + methodName + "]";
    }
    llvm::Value *codegen(llvm::IRBuilder<>* builder,
                         llvm::LLVMContext* context,
                         llvm::BasicBlock* entryBlock,
                         std::map<std::string, llvm::Value*>* namedValues,
                         llvm::Module* module,
                         map<string, string>* objectTypes,
                         map<string, ClassData>* classes) override;
};


#endif //STELLAR_ASTMETHODCALL_H
