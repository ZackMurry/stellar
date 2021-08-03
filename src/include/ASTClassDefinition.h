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

VariableType mapVariableTypeToGenericTypes(const VariableType& v, const vector<VariableType>& genericTypes, const vector<VariableType>& genericUsage);

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
    llvm::Value* codegen(CodegenData data) override;
    ASTNodeType getType() override {
        return AST_CLASS_DEFINITION;
    }
};


#endif //STELLAR_ASTCLASSDEFINITION_H
