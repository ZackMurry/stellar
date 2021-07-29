//
// Created by zack on 7/17/21.
//

#include "parser.h"
#include <string>
#ifndef STELLAR_ASTMETHODCALL_H
#define STELLAR_ASTMETHODCALL_H


class ASTMethodCall : public ASTNode {
    ASTNode* object;
    string methodName;
    vector<ASTNode*> args;
public:
    ASTMethodCall(ASTNode* object, string methodName, vector<ASTNode*> args) : object(object), methodName(move(methodName)), args(move(args)) {}
    string toString() override {
        return "[METHOD_CALL: object: " + object->toString() + " method: " + methodName + "]";
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
