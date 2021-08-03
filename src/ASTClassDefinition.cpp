// // Created by zack on 7/15/21.
//

#include "include/ASTClassDefinition.h"

llvm::Type* getLLVMGenericTypeByVariableType(const VariableType& v, map<string, ClassData>* classes, const vector<VariableType>& genericTypes, const vector<VariableType>& genericUsage, llvm::LLVMContext* context, llvm::Type* classType, const string& className) {
    string genericString = convertVariableTypeToString(v);
    cout << "genericString: " << genericString << ": " << classes->count(genericString) << ": " << classes->count("ListNode<i32>") << endl;
    int ivt = getPrimitiveVariableTypeFromString(genericString);
    if (ivt != -1) {
        return getLLVMTypeByPrimitiveVariableType((PrimitiveVariableType) ivt, context);
    } else if (classes->count(genericString)) {
        return llvm::PointerType::get(classes->at(genericString).type, 0);
    } else {
        // Check generic names
        for (int i = 0; i < genericTypes.size(); i++) {
            if (genericTypes.at(i).type == v.type) {
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

llvm::Value* ASTClassDefinition::codegen(llvm::IRBuilder<> *builder,
                                         llvm::LLVMContext *context,
                                         llvm::BasicBlock *entryBlock,
                                         map<string, llvm::Value *> *namedValues,
                                         llvm::Module *module,
                                         map<string, string>* objectTypes,
                                         map<string, ClassData>* classes) {
    if (genericTypes.empty()) {
        auto classType = llvm::StructType::create(*context, name);
        vector<llvm::Type*> fieldLLVMTypes;
        vector<ClassFieldType> llvmFields;
        for (const auto& field : fields) {
            cout << "Processing field " << field.name << endl;
            int variableType = getPrimitiveVariableTypeFromString(field.type.type);
            if (variableType != -1) {
                auto t = getLLVMTypeByPrimitiveVariableType((PrimitiveVariableType) variableType, context);
                fieldLLVMTypes.push_back(t);
                llvmFields.push_back({field.name, "", t });
            } else if (classes->count(field.type.type)) {
                auto t = llvm::PointerType::getUnqual(classes->at(field.type.type).type);
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
        classes->insert({ name, {classType, llvmFields } });

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
                    llvmType = getLLVMTypeByPrimitiveVariableType((PrimitiveVariableType) ivt, context);
                } else if (classes->count(genericType)) {
                    llvmType = llvm::PointerType::getUnqual(classes->at(genericType).type);
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
                returnType = getLLVMTypeByPrimitiveVariableType((PrimitiveVariableType) rtt, context);
            } else if (classes->count(genericReturnType)) {
                returnType = llvm::PointerType::get(classes->at(genericReturnType).type, 0);
            } else {
                cerr << "Error: invalid return type " << genericReturnType << endl;
                exit(EXIT_FAILURE);
            }
            llvm::FunctionType* ft = llvm::FunctionType::get(returnType, argTypes, false);
            auto func = llvm::Function::Create(ft, llvm::Function::ExternalLinkage, method.second->name, *module);
            unsigned index = 0;
            for (auto &arg : func->args()) {
                arg.setName(method.second->args[index++]->name);
            }
        }
        map<string, llvm::Function*> llvmMethods;
        for (const auto& method : methods) {
            auto m = (llvm::Function*) method.second->codegen(builder, context, entryBlock, namedValues, module, objectTypes, classes);
            llvmMethods.insert({ name, m });
        }
        classes->erase(name);
        classes->insert({ name, {classType, llvmFields, llvmMethods } });
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
        auto classType = llvm::StructType::create(*context, genericClassName);
        classes->insert({ genericClassName, {classType} });
        cout << "classes->count(\"ListNode<i32>\"): " << classes->count("ListNode<i32>") << endl;
        for (int i = 0; i < g.size(); i++)  {
            auto genericValue = convertVariableTypeToString(g.at(i));
            string genericType = convertVariableTypeToString(genericTypes.at(i));
            if (classes->count(genericValue)) {
                auto llvmType = llvm::PointerType::getUnqual(classes->at(genericValue).type);
                cout << "Adding temp class " << genericType << endl;
                classes->insert({ genericType, { llvmType } });
            } else if (genericValue == name) {
                auto llvmType = llvm::PointerType::getUnqual(classType);
                classes->insert({ genericType, { llvmType } });
            } else if (getPrimitiveVariableTypeFromString(genericValue) == -1) {
                cerr << "Error: expected type for generic type but instead found " << genericValue << endl;
                exit(EXIT_FAILURE);
            }
        }
        vector<llvm::Type*> fieldLLVMTypes;
        vector<ClassFieldType> llvmFields;
        for (const auto& field : fields) {
            cout << "Processing field " << field.name << endl;
            auto fieldType = getLLVMGenericTypeByVariableType(field.type, classes, genericTypes, g, context, classType,
                                                              name);
            if (fieldType == nullptr) {
                cerr << "Error: expected type for field type but instead found " << convertVariableTypeToString(field.type) << endl;
                exit(EXIT_FAILURE);
            }
            fieldLLVMTypes.push_back(fieldType);
            llvmFields.push_back({ field.name, name, fieldType });
        }
        classType->setBody(fieldLLVMTypes);
        classes->erase(genericClassName);
        classes->insert({ genericClassName, {classType, llvmFields } });

        // Generate stubs for methods so that they can recursively call themselves, call those defined after them, etc
        for (const auto& method : methods) {
            // Prepend class name to function name
            method.second->name = genericClassName + "__" + method.first;

            // Add "this" arg for reference to the class
            method.second->args.push_back(new ASTVariableDefinition("this", { name, genericTypes }));
            cout << "Method args size 0: " << method.second->args.size() << endl;
            vector<llvm::Type*> argTypes;
            for (const auto& arg : method.second->args) {
                auto argType = getLLVMGenericTypeByVariableType(arg->type, classes, genericTypes, g, context, classType, name);
                if (!argType) {
                    cerr << "Error: unknown argument type " << convertVariableTypeToString(arg->type) << endl;
                    exit(EXIT_FAILURE);
                }
                argTypes.push_back(argType);
            }
            cout << "method.second->returnType: " << method.second->returnType.type << endl;
            auto returnType = getLLVMGenericTypeByVariableType(method.second->returnType, classes, genericTypes, g,
                                                               context, classType, name);
            if (!returnType) {
                cerr << "Error: invalid method return type " << method.second->returnType.type << endl;
                exit(EXIT_FAILURE);
            }
//            int rtt = getPrimitiveVariableTypeFromString(method.second->returnType);
//            if (rtt != -1) {
//                returnType = getLLVMTypeByPrimitiveVariableType((PrimitiveVariableType) rtt, context);
//            } else if (classes->count(method.second->returnType)) {
//                returnType = llvm::PointerType::get(classes->at(method.second->returnType).type, 0);
//            } else {
//                // Check generic names
//                for (int i = 0; i < genericTypes.size(); i++) {
//                    if (genericTypes.at(i) == method.second->returnType) {
//                        string genericValue = g.at(i);
//                        int vt = getPrimitiveVariableTypeFromString(genericValue);
//                        if (vt != -1) {
//                            returnType = getLLVMTypeByPrimitiveVariableType((PrimitiveVariableType) vt, context);
//                        } else if (classes->count(genericValue)) {
//                            returnType = llvm::PointerType::getUnqual(classes->at(method.second->returnType).type);
//                        } else if (genericValue == name) {
//                            returnType = llvm::PointerType::getUnqual(classType);
//                        }
//                        break;
//                    }
//                }
//                if (!returnType) {
//                    cerr << "Error: invalid method return type " << method.second->returnType << endl;
//                    exit(EXIT_FAILURE);
//                }
//            }
            llvm::FunctionType* ft = llvm::FunctionType::get(returnType, argTypes, false);
            auto func = llvm::Function::Create(ft, llvm::Function::ExternalLinkage, method.second->name, *module);
            unsigned index = 0;
            for (auto &arg : func->args()) {
                arg.setName(method.second->args[index++]->name);
            }
        }
        map<string, llvm::Function*> llvmMethods;
        for (const auto& method : methods) {
            cout << "Method args size: " << method.second->args.size() << endl;
            auto m = (llvm::Function*) method.second->codegen(builder, context, entryBlock, namedValues, module, objectTypes, classes);
            llvmMethods.insert({ name, m });
            method.second->args.pop_back();
        }
        classes->erase(genericClassName);
        classes->insert({ genericClassName, {classType, llvmFields, llvmMethods } });
    }
    return nullptr;
}
