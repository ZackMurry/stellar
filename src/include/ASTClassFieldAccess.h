//
// Created by zack on 7/15/21.
//

#include "parser.h"
#include <string>
#ifndef STELLAR_ASTCLASSFIELDACCESS_H
#define STELLAR_ASTCLASSFIELDACCESS_H

using namespace std;

class ASTClassFieldAccess : public ASTNode {
    string identifier;
    string fieldName;
public:
    ASTClassFieldAccess(string identifier, string fieldName) : identifier(move(identifier)), fieldName(move(fieldName)) {}
    string toString() override {
        return "[CLASS_FIELD_ACC: field number " + fieldName + "]";
    }
    llvm::Value* codegen(llvm::IRBuilder<>* builder,
                         llvm::LLVMContext* context,
                         llvm::BasicBlock* entryBlock,
                         map<string, llvm::Value*>* namedValues,
                         llvm::Module* module,
                         map<string, string>* objectTypes,
                         map<string, ClassData>* classes) override;
};


#endif //STELLAR_ASTCLASSFIELDACCESS_H
