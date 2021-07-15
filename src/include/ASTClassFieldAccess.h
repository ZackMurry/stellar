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
    llvm::Type* classType;
    int fieldNumber;
public:
    ASTClassFieldAccess(string identifier, llvm::Type* classType, int fieldNumber) : identifier(move(identifier)), classType(classType), fieldNumber(fieldNumber) {}
    string toString() override {
        return "[CLASS_FIELD_ACC: field number " + to_string(fieldNumber) + "]";
    }
    llvm::Value* codegen(llvm::IRBuilder<>* builder,
                         llvm::LLVMContext* context,
                         llvm::BasicBlock* entryBlock,
                         map<string, llvm::Value*>* namedValues,
                         llvm::Module* module) override;
};


#endif //STELLAR_ASTCLASSFIELDACCESS_H
