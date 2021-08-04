// // Created by zack on 7/15/21.
//

#include "include/ASTClassDefinition.h"

VariableType mapVariableTypeToGenericTypes(VariableType v, const vector<VariableType>& genericTypes, const vector<VariableType>& genericUsage) {
    for (int i = 0; i < genericTypes.size(); i++) {
        if (v.type == genericTypes.at(i).type) {
            cout << "Mapped generic type " << v.type << " to " << convertVariableTypeToString(genericUsage.at(i)) << endl;
            v = genericUsage.at(i);
            break;
        }
    }
    for (auto & genericType : v.genericTypes) {
        genericType = mapVariableTypeToGenericTypes(genericType, genericTypes, genericUsage);
    }
    return v;
}

llvm::Type* getLLVMGenericTypeByVariableType(const VariableType& v, map<string, ClassData>* classes, const vector<VariableType>& genericTypes, const vector<VariableType>& genericUsage, llvm::LLVMContext* context, llvm::Type* classType, const string& className) {
    cout << convertVariableTypeToString(v) << "; Num generics: " << v.genericTypes.size() << endl;
    VariableType cv = mapVariableTypeToGenericTypes(v, genericTypes, genericUsage);
    cout << "Num generics: " << cv.genericTypes.size() << endl;
    string genericString = convertVariableTypeToString(cv);
    cout << "genericString: " << genericString << ": " << classes->count(genericString) << ": " << classes->count("ListNode<i32>") << endl;
    int ivt = getPrimitiveVariableTypeFromString(genericString);
    if (ivt != -1) {
        return getLLVMTypeByPrimitiveVariableType((PrimitiveVariableType) ivt, context);
    } else if (classes->count(genericString)) {
        return llvm::PointerType::get(classes->at(genericString).type, 0);
    } else {
        // Check generic names
        for (int i = 0; i < genericTypes.size(); i++) {
            if (genericTypes.at(i).type == cv.type) {
                string genericValue = genericUsage.at(i).type;
                int vt = getPrimitiveVariableTypeFromString(genericValue);
                if (vt != -1) {
                    return getLLVMTypeByPrimitiveVariableType((PrimitiveVariableType) vt, context);
                } else if (classes->count(genericValue)) {
                    return llvm::PointerType::getUnqual(classes->at(genericValue).type);
                } else if (genericValue == className) {
                    return llvm::PointerType::getUnqual(classType);
                }
            }
        }
        return nullptr;
    }
}

llvm::Value* ASTClassDefinition::codegen(CodegenData data) {
    cout << "Codegen for class " << name << endl;
    if (genericTypes.empty()) {
        auto classType = llvm::StructType::create(*data.context, name);
        vector<llvm::Type*> fieldLLVMTypes;
        vector<ClassFieldType> llvmFields;
        for (const auto& field : fields) {
            cout << "Processing field " << field.name << endl;
            int variableType = getPrimitiveVariableTypeFromString(field.type.type);
            if (variableType != -1) {
                auto t = getLLVMTypeByPrimitiveVariableType((PrimitiveVariableType) variableType, data.context);
                fieldLLVMTypes.push_back(t);
                llvmFields.push_back({field.name, "", t });
            } else if (data.classes->count(field.type.type)) {
                auto t = llvm::PointerType::getUnqual(data.classes->at(field.type.type).type);
                fieldLLVMTypes.push_back(t);
                llvmFields.push_back({field.name, field.type.type, t });
            } else if (field.type.type == name) {
                auto t = llvm::PointerType::getUnqual(classType);
                fieldLLVMTypes.push_back(t);
                llvmFields.push_back({ field.name, name, t });
            } else {
                cerr << "Error: expected type for field type but instead found " << field.type.type << endl;
                exit(EXIT_FAILURE);
            }
        }
        classType->setBody(fieldLLVMTypes);
        data.classes->insert({ name, {classType, llvmFields } });

        // Generate stubs for methods so that they can recursively call themselves, call those defined after them, etc
        for (const auto& method : methods) {
            // Prepend class name to function name
            method.second->name = name + "__" + method.first;

            // Add "this" arg for reference to the class
            method.second->args.push_back(new ASTVariableDefinition("this", { name, genericTypes }));
            vector<llvm::Type*> argTypes;
            for (const auto& arg : method.second->args) {
                auto genericType = convertVariableTypeToString(arg->type);
                llvm::Type* llvmType;
                int ivt = getPrimitiveVariableTypeFromString(genericType);
                if (ivt != -1) {
                    llvmType = getLLVMTypeByPrimitiveVariableType((PrimitiveVariableType) ivt, data.context);
                } else if (data.classes->count(genericType)) {
                    llvmType = llvm::PointerType::getUnqual(data.classes->at(genericType).type);
                } else {
                    cerr << "Error: unknown type " << genericType << endl;
                    exit(EXIT_FAILURE);
                }
                if (llvmType == nullptr) {
                    cerr << "Error mapping variable type to LLVM type" << endl;
                    exit(EXIT_FAILURE);
                }
                argTypes.push_back(llvmType);
            }
            auto genericReturnType = convertVariableTypeToString(method.second->returnType);
            llvm::Type* returnType;
            int rtt = getPrimitiveVariableTypeFromString(genericReturnType);
            if (rtt != -1) {
                returnType = getLLVMTypeByPrimitiveVariableType((PrimitiveVariableType) rtt, data.context);
            } else if (data.classes->count(genericReturnType)) {
                returnType = llvm::PointerType::get(data.classes->at(genericReturnType).type, 0);
            } else {
                cerr << "Error: invalid return type " << genericReturnType << endl;
                exit(EXIT_FAILURE);
            }
            llvm::FunctionType* ft = llvm::FunctionType::get(returnType, argTypes, false);
            auto func = llvm::Function::Create(ft, llvm::Function::ExternalLinkage, method.second->name, *data.module);
            unsigned index = 0;
            for (auto &arg : func->args()) {
                arg.setName(method.second->args[index++]->name);
            }
        }
        map<string, llvm::Function*> llvmMethods;
        for (const auto& method : methods) {
            auto m = (llvm::Function*) method.second->codegen(data);
            llvmMethods.insert({ name, m });
        }
        data.classes->erase(name);
        data.classes->insert({ name, {classType, llvmFields, llvmMethods } });
        return nullptr;
    }

    // Generic classes

    cout << "Num generic usages: " << genericUsages.size() << endl;
    for (const auto& g : genericUsages) {
        string genericClassName = name + "<";
        for (int i = 0; i < g.size(); i++) {
            auto genericValue = g.at(i);
            cout << "Generic usage " << i << ":" << genericValue.type << endl;
            if (i != 0) {
                genericClassName += ",";
            }
            genericClassName += convertVariableTypeToString(genericValue);
        }
        genericClassName += ">";
        cout << "Generic name: " << genericClassName << endl;
        auto classType = llvm::StructType::create(*data.context, genericClassName);
        data.classes->insert({ genericClassName, {classType} });
        for (int i = 0; i < genericTypes.size(); i++) {
            data.generics->insert({ genericTypes.at(i).type, g.at(i) });
        }
        for (int i = 0; i < g.size(); i++)  {
            auto genericValue = convertVariableTypeToString(g.at(i));
            string genericType = convertVariableTypeToString(genericTypes.at(i));
            if (data.classes->count(genericValue)) {
                auto llvmType = llvm::PointerType::getUnqual(data.classes->at(genericValue).type);
                cout << "Adding temp class " << genericType << endl;
                data.classes->insert({ genericType, { llvmType } });
            } else if (genericValue == name) {
                auto llvmType = llvm::PointerType::getUnqual(classType);
                data.classes->insert({ genericType, { llvmType } });
            } else if (getPrimitiveVariableTypeFromString(genericValue) == -1) {
                cerr << "Error: expected type for generic type but instead found " << genericValue << endl;
                exit(EXIT_FAILURE);
            }
        }
        vector<llvm::Type*> fieldLLVMTypes;
        vector<ClassFieldType> llvmFields;
        for (const auto& field : fields) {
            cout << "Processing field " << field.name << endl;
            auto fieldType = getLLVMGenericTypeByVariableType(field.type, data.classes, genericTypes, g, data.context, classType,
                                                              name);
            if (fieldType == nullptr) {
                cerr << "Error: expected type for field type but instead found " << convertVariableTypeToString(field.type) << endl;
                exit(EXIT_FAILURE);
            }
            fieldLLVMTypes.push_back(fieldType);
            llvmFields.push_back({ field.name, name, fieldType });
        }
        classType->setBody(fieldLLVMTypes);
        data.classes->erase(genericClassName);
        data.classes->insert({ genericClassName, {classType, llvmFields } });

        // Generate stubs for methods so that they can recursively call themselves, call those defined after them, etc
        for (const auto& method : methods) {
            // Prepend class name to function name
            method.second->name = genericClassName + "__" + method.first;

            // Add "this" arg for reference to the class
            method.second->args.push_back(new ASTVariableDefinition("this", { name, genericTypes }));
            cout << "Method args size 0: " << method.second->args.size() << endl;
            vector<llvm::Type*> argTypes;
            for (const auto& arg : method.second->args) {
                auto argType = getLLVMGenericTypeByVariableType(arg->type, data.classes, genericTypes, g, data.context, classType, name);
                if (!argType) {
                    cerr << "Error: unknown argument type " << convertVariableTypeToString(arg->type) << endl;
                    exit(EXIT_FAILURE);
                }
                argTypes.push_back(argType);
            }
            cout << "method.second->returnType: " << convertVariableTypeToString(method.second->returnType) << endl;
            auto returnType = getLLVMGenericTypeByVariableType(method.second->returnType, data.classes, genericTypes, g,
                                                               data.context, classType, name);
            if (!returnType) {
                cerr << "Error: invalid method return type " << method.second->returnType.type << endl;
                exit(EXIT_FAILURE);
            }
            llvm::FunctionType* ft = llvm::FunctionType::get(returnType, argTypes, false);
            auto func = llvm::Function::Create(ft, llvm::Function::ExternalLinkage, method.second->name, *data.module);
            unsigned index = 0;
            for (auto &arg : func->args()) {
                arg.setName(method.second->args[index++]->name);
            }
        }
        map<string, llvm::Function*> llvmMethods;
        for (const auto& method : methods) {
            cout << "Method args size: " << method.second->args.size() << endl;
            auto m = (llvm::Function*) method.second->codegen(data);
            llvmMethods.insert({ name, m });
            method.second->args.pop_back();
        }
        data.classes->erase(genericClassName);
        data.classes->insert({ genericClassName, {classType, llvmFields, llvmMethods } });
        data.generics->clear();
    }
    return nullptr;
}
