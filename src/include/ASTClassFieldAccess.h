//
// Created by zack on 7/15/21.
//

#include "parser.h"
#include <string>
#ifndef STELLAR_ASTCLASSFIELDACCESS_H
#define STELLAR_ASTCLASSFIELDACCESS_H

using namespace std;

class ASTClassFieldAccess : public ASTNode {
public:
    ASTNode* object;
    string fieldName;
    ASTClassFieldAccess(ASTNode* object, string fieldName) : object(object), fieldName(move(fieldName)) {}
    string toString() override {
        return "[CLASS_FIELD_ACC: " + object->toString() + " field: " + fieldName + "]";
    }
    llvm::Value* codegen(llvm::IRBuilder<>* builder,
                         llvm::LLVMContext* context,
                         llvm::BasicBlock* entryBlock,
                         map<string, llvm::Value*>* namedValues,
                         llvm::Module* module,
                         map<string, string>* objectTypes,
                         map<string, ClassData>* classes) override;
    ASTNodeType getType() override {
        return AST_CLASS_FIELD_ACCESS;
    }
};


#endif //STELLAR_ASTCLASSFIELDACCESS_H
