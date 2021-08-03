//
// Created by zack on 8/3/21.
//
#include "include/analysis.h"
#include "include/ASTClassDefinition.h"
#include "include/ASTClassInstantiation.h"
#include "include/ASTForExpression.h"
#include "include/ASTIfStatement.h"
#include "include/ASTNewExpression.h"
#include "include/ASTVariableDeclaration.h"
#include "include/ASTVariableDefinition.h"
#include "include/ASTWhileExpression.h"

map<string, ASTClassDefinition*> classDefs;

void addGenericUsageIfNotPresent(ASTClassDefinition* classDef, vector<VariableType> genericTypes) {
    if (classDef->genericTypes.size() != genericTypes.size()) {
        cerr << "Error: wrong number of generic types for class " << classDef->name << " (expected " << classDef->genericTypes.size() << " but got " << genericTypes.size() << ")" << endl;
        exit(EXIT_FAILURE);
    }
    if (genericTypes.empty()) {
        return;
    }
    for (const auto& usage : classDef->genericUsages) {
        bool isMatch = true;
        for (int i = 0; i < usage.size(); i++) {
            // todo: nested generics
            if (usage.at(i).type != genericTypes.at(i).type) {
                isMatch = false;
                break;
            }
        }
        if (isMatch) {
            cout << "Found match for generic types" << endl;
            return;
        }
    }
    for (int i = 0; i < genericTypes.size(); i++) {
        cout << "generic usage " << i << ": " << genericTypes.at(i).type << endl;
    }
    classDef->genericUsages.push_back(genericTypes);
}

void analyzeNode(ASTNode* node) {
    auto type = node->getType();
    if (type == AST_CLASS_DEFINITION) {
        auto classNode = dynamic_cast<ASTClassDefinition*>(node);
        classDefs.insert({ classNode->name, classNode });
    } else if (type == AST_CLASS_INSTANTIATION) {
        auto instNode = dynamic_cast<ASTClassInstantiation*>(node);
        addGenericUsageIfNotPresent(classDefs.at(instNode->className), instNode->genericTypes);
    } else if (type == AST_NEW_EXPRESSION) {
        auto newNode = dynamic_cast<ASTNewExpression*>(node);
        cout << "New expression: " << newNode->genericTypes.size() << endl;
        addGenericUsageIfNotPresent(classDefs.at(newNode->className), newNode->genericTypes);
    } else if (type == AST_VARIABLE_DEFINITION) {
        auto defNode = dynamic_cast<ASTVariableDefinition*>(node);
        if (classDefs.count(convertVariableTypeToString(defNode->type))) {
            addGenericUsageIfNotPresent(classDefs.at(defNode->type.type), defNode->type.genericTypes);
        }
    } else if (type == AST_VARIABLE_DECLARATION) {
        auto declNode = dynamic_cast<ASTVariableDeclaration*>(node);
        cout << "Variable declaration " << declNode->type.type << endl;
        if (classDefs.count(convertVariableTypeToString(declNode->type))) {
            addGenericUsageIfNotPresent(classDefs.at(declNode->type.type), declNode->type.genericTypes);
        }
        analyzeNode(declNode->value);
    } else if (type == AST_IF_STATEMENT) {
        auto ifNode = dynamic_cast<ASTIfStatement*>(node);
        for (const auto& n : ifNode->ifBody) {
            analyzeNode(n);
        }
        for (const auto& n : ifNode->elseBody) {
            analyzeNode(n);
        }
        analyzeNode(ifNode->condition);
    } else if (type == AST_FOR_EXPRESSION) {
        auto forNode = dynamic_cast<ASTForExpression*>(node);
        for (const auto& n : forNode->body) {
            analyzeNode(n);
        }
        analyzeNode(forNode->initializer);
        analyzeNode(forNode->condition);
        analyzeNode(forNode->action);
    } else if (type == AST_WHILE_EXPRESSION) {
        auto whileNode = dynamic_cast<ASTWhileExpression*>(node);
        for (const auto& n : whileNode->body) {
            analyzeNode(n);
        }
        analyzeNode(whileNode->condition);
    }
}

void analyze(const vector<ASTNode*>& nodes) {
    classDefs.clear();
    for (const auto& node : nodes) {
        analyzeNode(node);
    }
}
