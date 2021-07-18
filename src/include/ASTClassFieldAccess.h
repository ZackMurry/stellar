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
    vector<string> fieldNames;
public:
    ASTClassFieldAccess(string identifier, vector<string> fieldNames) : identifier(move(identifier)), fieldNames(move(fieldNames)) {}
    string toString() override {
        return "[CLASS_FIELD_ACC: num fields: " + to_string(fieldNames.size()) + "]";
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
