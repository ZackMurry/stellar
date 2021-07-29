//
// Created by zack on 7/15/21.
//

#include "parser.h"
#include <string>
#ifndef STELLAR_ASTCLASSFIELDSTORE_H
#define STELLAR_ASTCLASSFIELDSTORE_H

using namespace std;

class ASTClassFieldStore : public ASTNode {
    ASTNode* object;
    string fieldName;
    ASTNode* value;
public:
    ASTClassFieldStore(ASTNode* object, string fieldName, ASTNode* value) : object(object), fieldName(move(fieldName)), value(value) {}
    string toString() override {
        return "[CLASS_FIELD_STO: object: " + object->toString() + " field: " + fieldName + " value: " + value->toString() + "]";
    }
    llvm::Value* codegen(llvm::IRBuilder<>* builder,
                         llvm::LLVMContext* context,
                         llvm::BasicBlock* entryBlock,
                         map<string, llvm::Value*>* namedValues,
                         llvm::Module* module,
                         map<string, string>* objectTypes,
                         map<string, ClassData>* classes) override;
};


#endif //STELLAR_ASTCLASSFIELDSTORE_H
