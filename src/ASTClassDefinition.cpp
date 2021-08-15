// // Created by zack on 7/15/21.
//

#include <include/ASTStringExpression.h>
#include "include/ASTClassDefinition.h"
#include <set>

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
    cout << "Num generic usages: " << genericUsages.size() << endl;
    for (const auto& g : genericUsages) {
        string genericClassName = name;
        if (!genericTypes.empty()) {
            genericClassName += "<";
            for (int i = 0; i < g.size(); i++) {
                auto genericValue = g.at(i);
                if (i != 0) {
                    genericClassName += ",";
                }
                genericClassName += convertVariableTypeToString(genericValue);
            }
            genericClassName += ">";
        }
        cout << "Generic name: " << genericClassName << endl;
        if (!data.classes->count(genericClassName)) {
            cerr << "Internal error: expected class definition to already be defined by analysis" << endl;
            exit(EXIT_FAILURE);
        }
        auto classType = (llvm::StructType*) data.classes->at(genericClassName).type;
        cout << "is class pointer? " << classType->isPointerTy() << endl;
        cout << "Class struct name: " << classType->getStructName().str() << endl;
//        for (const auto& ist : *data.classes) {
//            if (genericClassName == ist->getName().str()) {
//                classType = ist;
//                break;
//            }
//            cout << "Doesn't match " << ist->getName().str() << endl;
//        }
        for (int i = 0; i < genericTypes.size(); i++) {
            data.generics->insert({ genericTypes.at(i).type, g.at(i) });
        }
        for (int i = 0; i < g.size(); i++)  {
            auto genericValue = convertVariableTypeToString(g.at(i));
            string genericType = convertVariableTypeToString(genericTypes.at(i));
            if (data.classes->count(genericValue)) {
                auto llvmType = llvm::PointerType::getUnqual(data.classes->at(genericValue).type);
                data.classes->insert({ genericType, { llvmType } });
            } else if (genericValue == name) {
                auto llvmType = llvm::PointerType::getUnqual(classType);
                data.classes->insert({ genericType, { llvmType } });
            } else if (getPrimitiveVariableTypeFromString(genericValue) != -1) {
                data.classes->insert({ genericType, {getLLVMTypeByPrimitiveVariableType(
                        (PrimitiveVariableType) getPrimitiveVariableTypeFromString(genericValue), data.context) } });
            } else {
                cerr << "Error: expected type for generic type but instead found " << genericValue << endl;
                exit(EXIT_FAILURE);
            }
        }
        vector<llvm::Type*> fieldLLVMTypes;
        vector<ClassFieldType> llvmFields;


        // Create vtable
        auto vtable = llvm::StructType::create(*data.context, genericClassName + "_vtable_type");
        vector<llvm::Type*> vtableBody;
        vector<string> methodOrder;

        // Generate stubs for methods so that they can recursively call themselves, call those defined after them, etc
        vector<llvm::Constant*> vtableArr;
        map<string, llvm::Function*> llvmMethods;
        set<string> definedMethods;
        map<string, bool> isMethodOverriding;
        for (const auto& method : methods) {
            definedMethods.insert(method->name);
        }
        auto vtableType = llvm::PointerType::getUnqual(vtable);
        fieldLLVMTypes.push_back(vtableType);
        string pcStr;
        if (parentClass) {
            pcStr = convertVariableTypeToString(mapVariableTypeToGenericTypes(*parentClass, genericTypes, g));
            // Add parent class's fields
            if (!data.classes->count(pcStr)) {
                cerr << "Error: unknown parent class of " << genericClassName << ": " << pcStr << endl;
                exit(EXIT_FAILURE);
            }
            auto parentFields = data.classes->at(pcStr).fields;
            cout << parentFields.size() << " parent fields" << endl;
            for (const auto& field : parentFields) {
                cout << "Processing parent field " << field.name << endl;
                fieldLLVMTypes.push_back(field.type);
                llvmFields.push_back({ field.name, name, field.type });
            }
        }
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
        data.classes->insert({ genericClassName, {classType, llvmFields, methodOrder, map<string, llvm::Function*>(), parentClass, vtableType } });
        if (parentClass) {
            cout << "Class has parent" << endl;
            auto cd = data.classes->at(pcStr);
            for (const auto &methodName : cd.methodOrder) {
                cout << name << " has method " << methodName << endl;
                if (definedMethods.count(methodName)) {
                    if (!cd.methodAttributes.at(methodName).isVirtual) {
                        cerr << "Error: illegal override of non-virtual method " << methodName << " of class " << pcStr << endl;
                        exit(EXIT_FAILURE);
                    }
                    if (!methodAttributes.at(methodName).isOverride) {
                        cerr << "Error: method " << methodName << " of class " << name << " has the same name as a method in its parent class. Please mark it as overriding or rename it." << endl;
                        exit(EXIT_FAILURE);
                    }
                    cout << "Skipping inherited method " << methodName << endl;
                    // Prepend class name to function name
                    ASTFunctionDefinition* method;
                    for (const auto& m : methods) {
                        if (m->name == methodName) {
                            method = m;
                            break;
                        }
                    }
                    if (!method) {
                        cerr << "Internal error: could not find overriding method" << endl;
                        exit(EXIT_FAILURE);
                    }
                    methodOrder.push_back(method->name);
                    isMethodOverriding.insert({ methodName, true });
                    method->name = genericClassName + "__" + method->name;

                    // Add "this" arg for reference to the class
                    method->args.push_back(new ASTVariableDefinition("this", { name, genericTypes }));
                    vector<llvm::Type*> argTypes;
                    for (const auto& arg : method->args) {
                        auto argType = getLLVMGenericTypeByVariableType(arg->type, data.classes, genericTypes, g, data.context, classType, name);
                        if (!argType) {
                            cerr << "Error: unknown argument type " << convertVariableTypeToString(arg->type) << endl;
                            exit(EXIT_FAILURE);
                        }
                        argTypes.push_back(argType);
                    }
                    auto returnType = getLLVMGenericTypeByVariableType(method->returnType, data.classes, genericTypes, g,
                                                                     data.context, classType, name);
                    if (!returnType) {
                        cerr << "Error: invalid method return type " << method->returnType.type << endl;
                        exit(EXIT_FAILURE);
                    }
                    llvm::FunctionType* ft = llvm::FunctionType::get(returnType, argTypes, false);
                    auto func = llvm::Function::Create(ft, llvm::Function::ExternalLinkage, method->name, *data.module);
                    unsigned index = 0;
                    for (auto &arg : func->args()) {
                        arg.setName(method->args[index++]->name);
                    }
                    auto c = (llvm::Function*) method->codegen(data);
                    vtableBody.push_back(c->getType());
                    vtableArr.push_back(c);
                    llvmMethods.insert({ methodName, c });
                    method->name = methodName;
                } else {
                    if (!isAbstract && cd.methodAttributes.at(methodName).isAbstract) {
                        cerr << "Error: an implementation of an abstract class must override all abstract methods (" << methodName << " is not overridden in " << name << ")" << endl;
                        exit(EXIT_FAILURE);
                    }
                    cout << "Inherited method " << methodName << endl;
                    methodOrder.push_back(methodName);
                    for (const auto& m : cd.methods) {
                        cout << "Inheritance: " << m.first << endl;
                    }
                    auto method = cd.methods.at(methodName);
                    llvmMethods.insert({methodName, method});
                    definedMethods.insert(methodName);
                    if (!cd.methodAttributes.at(methodName).isVirtual) {
                        // No need to add to vtable if the method won't be overridden
                        // However, a virtual method in the parent needs to be treated as virtual in the child
                        // in order to allow a grandchild to override it
                        methodAttributes.insert({ methodName, { false, false } });
                        continue;
                    }
                    methodAttributes.insert({ methodName, { true, false, cd.methodAttributes.at(methodName).isAbstract } });
                    vtableArr.push_back(method);
                    vtableBody.push_back(method->getType());
                }
            }
        }
        for (const auto& method : methods) {
            string methodName = method->name;
            if (isMethodOverriding.count(method->name)) {
                method->name = genericClassName + "__" + method->name;
                continue;
            }
            if (methodAttributes.at(method->name).isOverride) {
                cerr << "Error: illegal override of non-existing parent method" << endl;
                exit(EXIT_FAILURE);
            }
            // Prepend class name to function name
            methodOrder.push_back(method->name);
            method->name = genericClassName + "__" + method->name;

            // Add "this" arg for reference to the class
            method->args.push_back(new ASTVariableDefinition("this", { name, genericTypes }));
            vector<llvm::Type*> argTypes;
            for (const auto& arg : method->args) {
                auto argType = getLLVMGenericTypeByVariableType(arg->type, data.classes, genericTypes, g, data.context, classType, name);
                if (!argType) {
                    cerr << "Error: unknown argument type " << convertVariableTypeToString(arg->type) << endl;
                    exit(EXIT_FAILURE);
                }
                argTypes.push_back(argType);
            }
            auto returnType = getLLVMGenericTypeByVariableType(method->returnType, data.classes, genericTypes, g,
                                                               data.context, classType, name);
            if (!returnType) {
                cerr << "Error: invalid method return type " << method->returnType.type << endl;
                exit(EXIT_FAILURE);
            }
            llvm::FunctionType* ft = llvm::FunctionType::get(returnType, argTypes, false);
            if (methodAttributes.at(methodName).isVirtual) {
                vtableBody.push_back(llvm::PointerType::getUnqual(ft));
            }
            auto func = llvm::Function::Create(ft, llvm::Function::ExternalLinkage, method->name, *data.module);
            unsigned index = 0;
            for (auto &arg : func->args()) {
                arg.setName(method->args[index++]->name);
            }
        }
        vtable->setBody(vtableBody);
        fieldLLVMTypes.assign(0, vtableType);
        data.classes->erase(genericClassName);
        data.classes->insert({ genericClassName, {classType, llvmFields, methodOrder, map<string, llvm::Function*>(), parentClass, vtableType } });
        for (const auto& methodName : methodOrder) {
            if (isMethodOverriding.count(methodName)) {
                continue;
            }
            ASTFunctionDefinition* method = nullptr;
            for (const auto& m : methods) {
                if (m->name == genericClassName + "__" + methodName) {
                    method = m;
                    break;
                }
            }
            if (!method) {
                continue;
            }
            auto m = (llvm::Function*) method->codegen(data);
            string originalMethodName = method->name.substr(genericClassName.size() + 2, method->name.size() - (genericClassName.size() + 2));
            llvmMethods.insert({ originalMethodName, m });
            if (methodAttributes.at(methodName).isVirtual) {
                vtableArr.push_back(m);
            }
            method->args.pop_back();
            method->name = originalMethodName;
        }
        if (!vtableArr.empty()) {
            cout << name << " requires a vtable" << endl;
            auto vtableGlobal = new llvm::GlobalVariable(*data.module, vtable, false, llvm::GlobalValue::CommonLinkage,
                                                         nullptr, genericClassName + "_vtable");

            vtableGlobal->setInitializer(llvm::ConstantStruct::get(vtable, llvm::ArrayRef<llvm::Constant*>(vtableArr)));
            // For some reason, the vtable fields are initialized with nulls unless they are initialized in a function
            for (int i = 0; i < vtableArr.size(); i++) {
                auto gep = data.builder->CreateStructGEP(vtableGlobal, i);
                data.builder->CreateStore(vtableArr[i], gep);
            }
            data.classes->erase(genericClassName);
            data.classes->insert({ genericClassName, {classType, llvmFields, methodOrder, llvmMethods, parentClass, vtableType, vtableGlobal, methodAttributes, isAbstract } });
        } else {
            data.classes->erase(genericClassName);
            data.classes->insert({ genericClassName, {classType, llvmFields, methodOrder, llvmMethods, parentClass,
                                                      nullptr, nullptr, methodAttributes, isAbstract } });
        }
        data.generics->clear();
    }
    return nullptr;
}
