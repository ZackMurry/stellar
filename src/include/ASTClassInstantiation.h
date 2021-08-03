//
// Created by zack on 7/15/21.
//

#include "parser.h"
#include <string>
#ifndef STELLAR_ASTCLASSINSTANTIATION_H
#define STELLAR_ASTCLASSINSTANTIATION_H

using namespace std;

class ASTClassInstantiation : public ASTNode {
public:
    string className;
    string identifier;
    vector<ASTNode*> args;
    vector<VariableType> genericTypes;
    ASTClassInstantiation(string className, string identifier, vector<ASTNode*> args, vector<VariableType> genericTypes) : className(move(className)), identifier(move(identifier)), args(move(args)), genericTypes(move(genericTypes)) {}
    string toString() override {
        string s = "[CLASS_INST: type: " + className + " name: " + identifier + " num args: " + to_string(args.size()) + " generic types: [";
        for (const auto& gt : genericTypes) {
            s += convertVariableTypeToString(gt);
        }
        return s + "]]";
    }
    llvm::Value* codegen(CodegenData data) override;
    ASTNodeType getType() override {
        return AST_CLASS_INSTANTIATION;
    }
};


#endif //STELLAR_ASTCLASSINSTANTIATION_H
