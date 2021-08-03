//
// Created by zack on 7/15/21.
//

#include "parser.h"
#include "ASTFunctionDefinition.h"
#include <string>
#include <utility>
#ifndef STELLAR_ASTCLASSDEFINITION_H
#define STELLAR_ASTCLASSDEFINITION_H

using namespace std;

class ASTClassDefinition : public ASTNode {
public:
    string name;
    vector<ClassFieldDefinition> fields;
    map<string, ASTFunctionDefinition*> methods;
    vector<VariableType> genericTypes;
    vector<vector<VariableType>> genericUsages;
    explicit ASTClassDefinition(string name, vector<ClassFieldDefinition> fields, map<string, ASTFunctionDefinition*> methods, vector<VariableType> genericTypes) : name(move(name)), fields(move(fields)), methods(move(methods)), genericTypes(move(genericTypes)) {}
    string toString() override {
        string s = "[CLASS_DEF: " + name + " num fields: " + to_string(fields.size()) + " methods: [";
        for (const auto& method : methods) {
            s += method.second->toString();
        }
        return s + "]]";
    }
    llvm::Value* codegen(llvm::IRBuilder<>* builder,
                         llvm::LLVMContext* context,
                         llvm::BasicBlock* entryBlock,
                         map<string, llvm::Value*>* namedValues,
                         llvm::Module* module,
                         map<string, string>* objectTypes,
                         map<string, ClassData>* classes) override;
    ASTNodeType getType() override {
        return AST_CLASS_DEFINITION;
    }
};


#endif //STELLAR_ASTCLASSDEFINITION_H
