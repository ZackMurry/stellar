//
// Created by zack on 7/18/21.
//

#include "parser.h"
#ifndef STELLAR_ASTCLASSFIELDARRAYACCESS_H
#define STELLAR_ASTCLASSFIELDARRAYACCESS_H


class ASTClassFieldArrayAccess : public ASTNode {
    string identifier;
    string fieldName;
    ASTNode* index;
public:
    ASTClassFieldArrayAccess(string identifier, string fieldName, ASTNode* index) : identifier(move(identifier)), fieldName(move(fieldName)), index(index) {}
    string toString() override {
        return "[CLASS_FIELD_ARR_ACC: field: " + fieldName + " identifier: " + identifier + " index: " + index->toString() + "]";
    }
    llvm::Value* codegen(llvm::IRBuilder<>* builder,
                         llvm::LLVMContext* context,
                         llvm::BasicBlock* entryBlock,
                         map<string, llvm::Value*>* namedValues,
                         llvm::Module* module,
                         map<string, string>* objectTypes,
                         map<string, ClassData>* classes) override;
};


#endif //STELLAR_ASTCLASSFIELDARRAYACCESS_H
