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

VariableType mapVariableTypeToGenericTypes(VariableType v, const vector<VariableType>& genericTypes, const vector<VariableType>& genericUsage);

llvm::Type* getLLVMGenericTypeByVariableType(const VariableType& v, map<string, ClassData>* classes, const vector<VariableType>& genericTypes, const vector<VariableType>& genericUsage, llvm::LLVMContext* context, llvm::Type* classType, const string& className);
class ASTClassDefinition : public ASTNode {
public:
    string name;
    vector<ClassFieldDefinition> fields;
    vector<ASTFunctionDefinition*> methods;
    vector<VariableType> genericTypes;
    vector<vector<VariableType>> genericUsages;
    string parentClass; // Empty means none
    map<string, MethodAttributes> methodAttributes;
    bool isAbstract;
    ASTClassDefinition(string name,
                       vector<ClassFieldDefinition> fields,
                       vector<ASTFunctionDefinition*> methods,
                       vector<VariableType> genericTypes,
                       string parentClass,
                       map<string, MethodAttributes> methodAttributes,
                       bool isAbstract)
                       :
                       name(move(name)), fields(move(fields)), methods(move(methods)), genericTypes(move(genericTypes)),
                       parentClass(move(parentClass)), methodAttributes(move(methodAttributes)), isAbstract(isAbstract) {}
    string toString() override {
        string s = "[CLASS_DEF: " + name + " isAbstract: " + to_string(isAbstract) + " parent: [" + parentClass + "] generics: [";
        for (const auto& gt : genericTypes) {
            s += "[" + convertVariableTypeToString(gt) + "]";
        }
        s += "] generic usages: [";
        for (const auto& gu : genericUsages) {
            s += "[";
            for (const auto& g : gu) {
                s += "[" + convertVariableTypeToString(g) + "]";
            }
            s += "]";
        }
        s += "] fields: [";
        for (const auto& field : fields) {
            s += "[name: " + field.name + " type: " + convertVariableTypeToString(field.type) + "]";
        }
        s += "] methods: [";
        for (const auto& method : methods) {
            s += method->toString();
        }
        s += "] method attributes: [";
        for (const auto& ma : methodAttributes) {
            s += "[name: " + ma.first + " isVirtual: " + to_string(ma.second.isVirtual) + " isOverride: " + to_string(ma.second.isOverride) + " isAbstract: " + to_string(ma.second.isAbstract) + "]";
        }
        return s + "]";
    }
    llvm::Value* codegen(CodegenData data) override;
    ASTNodeType getType() override {
        return AST_CLASS_DEFINITION;
    }
};


#endif //STELLAR_ASTCLASSDEFINITION_H
