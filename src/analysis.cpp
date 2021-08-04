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

// Class links link generic type usages to other classes
// For example, in a class Box<T>, if it referred to a class ValueHolder<T>, the ValueHolder needs to have a codegen of type T
// Fmt: origin class to outgoing links, which are stored in a vector representing the indices of the generics needed
map<string, map<string, vector<int>>> classLinks;

bool areVariableTypesEqual(const VariableType& vt1, const VariableType& vt2) {
    if (vt1.type != vt2.type) {
        return false;
    }
    if (vt1.genericTypes.size() != vt2.genericTypes.size()) {
        return false;
    }
    for (int i = 0; i < vt1.genericTypes.size(); i++) {
        if (!areVariableTypesEqual(vt1.genericTypes.at(i), vt2.genericTypes.at(i))) {
            return false;
        }
    }
    return true;
}

void addGenericUsageIfNotPresent(const string& className, vector<VariableType> genericTypes) {
    cout << "Adding generic usage INP to " << className << endl;
    auto classDef = classDefs.at(className);
    if (classDef->genericTypes.size() != genericTypes.size()) {
        cerr << "Error: wrong number of generic types for class " << classDef->name << " (expected " << classDef->genericTypes.size() << " but got " << genericTypes.size() << ")" << endl;
        exit(EXIT_FAILURE);
    }
    for (const auto& gt : genericTypes) {
        if (!gt.genericTypes.empty()) {
            addGenericUsageIfNotPresent(gt.type, gt.genericTypes);
        }
    }
    for (const auto& usage : classDef->genericUsages) {
        bool isMatch = true;
        for (int i = 0; i < usage.size(); i++) {
            if (!areVariableTypesEqual(usage.at(i), genericTypes.at(i))) {
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
    if (classLinks.count(className)) {
        for (const auto& links : classLinks.at(className)) {
            cout << "Propagating generic usage to " << links.first << endl;
            vector<VariableType> relevantGenerics;
            for (const auto& i : links.second) {
                relevantGenerics.push_back(genericTypes.at(i));
            }
            addGenericUsageIfNotPresent(links.first, relevantGenerics);
        }
    }
}

void analyzeNode(ASTNode* node, string* parentClass) {
    auto type = node->getType();
    if (type == AST_CLASS_DEFINITION) {
        auto classNode = dynamic_cast<ASTClassDefinition*>(node);
        classDefs.insert({ classNode->name, classNode });
        for (const auto& field : classNode->fields) {
            cout << "Parsing field of type " << convertVariableTypeToString(field.type) << " (" << field.name << ") of class " << classNode->name << endl;
            if (field.type.type == classNode->name) {
                continue;
            }
            for (const auto& t : field.type.genericTypes) {
                for (int i = 0; i < classNode->genericTypes.size(); i++) {
                    auto gt = classNode->genericTypes.at(i);
                    if (t.type == gt.type) {
                        cout << "Generic used with generic type " << gt.type << endl;
                        cout << "Field.type.type: " << field.type.type << endl;
                        if (classLinks.count(classNode->name) && classLinks.at(classNode->name).count(field.type.type)) {
                            classLinks.at(classNode->name).at(field.type.type).push_back(i);
                        } else if (classLinks.count(classNode->name)) {
                            auto vec = vector<int>();
                            vec.push_back(i);
                            classLinks.at(classNode->name).insert({ field.type.type, vec });
                        } else {
                            auto vec = vector<int>();
                            vec.push_back(i);
                            auto m = map<string, vector<int>>();
                            m.insert({ field.type.type, vec });
                            classLinks.insert({ classNode->name, m });
                        }
                    }
                }
            }
        }
        if (!classNode->parentClass.empty()) {
            addGenericUsageIfNotPresent(classNode->parentClass, vector<VariableType>());
        }
    } else if (type == AST_CLASS_INSTANTIATION) {
        auto instNode = dynamic_cast<ASTClassInstantiation*>(node);
        addGenericUsageIfNotPresent(instNode->className, instNode->genericTypes);
    } else if (type == AST_NEW_EXPRESSION) {
        auto newNode = dynamic_cast<ASTNewExpression*>(node);
        cout << "New expression: " << newNode->classType.genericTypes.size() << endl;
        addGenericUsageIfNotPresent(newNode->classType.type, newNode->classType.genericTypes);
    } else if (type == AST_VARIABLE_DEFINITION) {
        auto defNode = dynamic_cast<ASTVariableDefinition*>(node);
        if (classDefs.count(convertVariableTypeToString(defNode->type))) {
            addGenericUsageIfNotPresent(defNode->type.type, defNode->type.genericTypes);
        }
    } else if (type == AST_VARIABLE_DECLARATION) {
        auto declNode = dynamic_cast<ASTVariableDeclaration*>(node);
        cout << "Variable declaration " << declNode->type.type << endl;
        if (classDefs.count(convertVariableTypeToString(declNode->type))) {
            addGenericUsageIfNotPresent(declNode->type.type, declNode->type.genericTypes);
        }
        analyzeNode(declNode->value, parentClass);
    } else if (type == AST_IF_STATEMENT) {
        auto ifNode = dynamic_cast<ASTIfStatement*>(node);
        for (const auto& n : ifNode->ifBody) {
            analyzeNode(n, parentClass);
        }
        for (const auto& n : ifNode->elseBody) {
            analyzeNode(n, parentClass);
        }
        analyzeNode(ifNode->condition, parentClass);
    } else if (type == AST_FOR_EXPRESSION) {
        auto forNode = dynamic_cast<ASTForExpression*>(node);
        for (const auto& n : forNode->body) {
            analyzeNode(n, parentClass);
        }
        analyzeNode(forNode->initializer, parentClass);
        analyzeNode(forNode->condition, parentClass);
        analyzeNode(forNode->action, parentClass);
    } else if (type == AST_WHILE_EXPRESSION) {
        auto whileNode = dynamic_cast<ASTWhileExpression*>(node);
        for (const auto& n : whileNode->body) {
            analyzeNode(n, parentClass);
        }
        analyzeNode(whileNode->condition, parentClass);
    }
}

void analyze(const vector<ASTNode*>& nodes) {
    classDefs.clear();
    classLinks.clear();
    for (const auto &node : nodes) {
        analyzeNode(node, nullptr);
    }
    for (const auto &cl : classLinks) {
        cout << cl.first << " has " << cl.second.size() << " class links" << endl;
        for (const auto &c : cl.second) {
            cout << cl.first << " is linked to " << c.first << endl;
        }
    }
    for (const auto &cd : classDefs) {
        cout << cd.first << " has " << cd.second->genericUsages.size() << " generic usages" << endl;
        for (const auto& gu : cd.second->genericUsages) {
            cout << "Generic usage: " << endl;
            for (const auto& g : gu) {
                cout << convertVariableTypeToString(g) << endl;
            }
        }
    }
}
