//
// Created by zack on 7/15/21.
//

#include "parser.h"
#include <string>
#ifndef STELLAR_ASTCLASSFIELDSTORE_H
#define STELLAR_ASTCLASSFIELDSTORE_H

using namespace std;

class ASTClassFieldStore : public ASTNode {
    vector<string> identifiers;
    string fieldName;
    ASTNode* value;
public:
    ASTClassFieldStore(vector<string> identifiers, string fieldName, ASTNode* value) : identifiers(move(identifiers)), fieldName(move(fieldName)), value(value) {}
    string toString() override {
        return "[CLASS_FIELD_STO: ident: " + to_string(identifiers.size()) + " field: " + fieldName + " value: " + value->toString() + "]";
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
