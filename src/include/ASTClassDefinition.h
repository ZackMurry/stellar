//
// Created by zack on 7/15/21.
//

#include "parser.h"
#include <string>
#ifndef STELLAR_ASTCLASSDEFINITION_H
#define STELLAR_ASTCLASSDEFINITION_H

using namespace std;

class ASTClassDefinition : public ASTNode {
    string name;
    map<string, string> fields;
    vector<string> fieldTypes;
public:
    explicit ASTClassDefinition(string name, map<string, string> fields, vector<string> fieldTypes) : name(move(name)), fields(move(fields)), fieldTypes(move(fieldTypes)) {}
    string toString() override {
        return "[CLASS_DEF: " + name + " num fields: " + to_string(fields.size()) + "]";
    }
    llvm::Value* codegen(llvm::IRBuilder<>* builder,
                         llvm::LLVMContext* context,
                         llvm::BasicBlock* entryBlock,
                         map<string, llvm::Value*>* namedValues,
                         llvm::Module* module,
                         map<string, string>* objectTypes,
                         map<string, ClassData>* classes) override;
};


#endif //STELLAR_ASTCLASSDEFINITION_H
