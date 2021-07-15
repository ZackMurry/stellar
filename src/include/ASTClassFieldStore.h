//
// Created by zack on 7/15/21.
//

#include "parser.h"
#include <string>
#ifndef STELLAR_ASTCLASSFIELDSTORE_H
#define STELLAR_ASTCLASSFIELDSTORE_H

using namespace std;

class ASTClassFieldStore : public ASTNode {
    string identifier;
    llvm::Type* classType;
    int fieldNumber;
    ASTNode* value;
public:
    ASTClassFieldStore(string identifier, llvm::Type* classType, int fieldNumber, ASTNode* value) : identifier(move(identifier)), classType(classType), fieldNumber(fieldNumber), value(value) {}
    string toString() override {
        return "[CLASS_FIELD_STO: ident: " + identifier + " field: " + to_string(fieldNumber) + " value: " + value->toString() + "]";
    }
    llvm::Value* codegen(llvm::IRBuilder<>* builder,
                         llvm::LLVMContext* context,
                         llvm::BasicBlock* entryBlock,
                         map<string, llvm::Value*>* namedValues,
                         llvm::Module* module) override;
};


#endif //STELLAR_ASTCLASSFIELDSTORE_H
