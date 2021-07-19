//
// Created by zack on 7/15/21.
//

#include "include/ASTClassDefinition.h"

llvm::Value* ASTClassDefinition::codegen(llvm::IRBuilder<> *builder,
                                         llvm::LLVMContext *context,
                                         llvm::BasicBlock *entryBlock,
                                         map<string, llvm::Value *> *namedValues,
                                         llvm::Module *module,
                                         map<string, string>* objectTypes,
                                         map<string, ClassData>* classes) {
    auto classType = llvm::StructType::create(*context, name);
    vector<llvm::Type*> fieldLLVMTypes;
    vector<ClassFieldType> llvmFields;
    for (const auto& field : fields) {
        cout << "Processing field " << field.name << endl;
        int variableType = getVariableTypeFromString(field.type);
        if (variableType != -1) {
            auto t = getLLVMTypeByVariableType((VariableType) variableType, context);
            fieldLLVMTypes.push_back(t);
            llvmFields.push_back({field.name, "", t });
        } else if (classes->count(field.type)) {
            auto t = llvm::PointerType::getUnqual(classes->at(field.type).type);
            fieldLLVMTypes.push_back(t);
            llvmFields.push_back({field.name, field.type, t });
        } else if (field.type == name) {
            auto t = llvm::PointerType::getUnqual(classType);
            fieldLLVMTypes.push_back(t);
            llvmFields.push_back({ field.name, name, t });
        } else {
            cerr << "Error: expected type for field type but instead found " << field.type << endl;
            exit(EXIT_FAILURE);
        }
    }
    classType->setBody(fieldLLVMTypes);
    classes->insert({ name, {classType, llvmFields } });

    // Generate stubs for methods so that they can recursively call themselves, call those defined after them, etc
    for (const auto& method : methods) {
        // Prepend class name to function name
        method.second->setName(name + "__" + method.first);

        // Add "t" arg for reference to the class
        method.second->addArg(new ASTVariableDefinition("t", name));
        vector<llvm::Type*> argTypes;
        for (const auto& arg : method.second->getArgs()) {
            llvm::Type* llvmType;
            int ivt = getVariableTypeFromString(arg->getType());
            if (ivt != -1) {
                llvmType = getLLVMTypeByVariableType((VariableType) ivt, context);
            } else if (classes->count(arg->getType())) {
                llvmType = llvm::PointerType::getUnqual(classes->at(arg->getType()).type);
            } else {
                cerr << "Error: unknown type " << arg->getType() << endl;
                exit(EXIT_FAILURE);
            }
            if (llvmType == nullptr) {
                cerr << "Error mapping variable type to LLVM type" << endl;
                exit(EXIT_FAILURE);
            }
            argTypes.push_back(llvmType);
        }
        llvm::Type* returnType;
        int rtt = getVariableTypeFromString(method.second->getReturnType());
        if (rtt != -1) {
            returnType = getLLVMTypeByVariableType((VariableType) rtt, context);
        } else if (classes->count(method.second->getReturnType())) {
            returnType = llvm::PointerType::get(classes->at(method.second->getReturnType()).type, 0);
        } else {
            cerr << "Error: invalid return type " << method.second->getReturnType() << endl;
            exit(EXIT_FAILURE);
        }
        llvm::FunctionType* ft = llvm::FunctionType::get(returnType, argTypes, false);
        auto func = llvm::Function::Create(ft, llvm::Function::ExternalLinkage, method.second->getName(), *module);
        unsigned index = 0;
        for (auto &arg : func->args()) {
            arg.setName(method.second->getArgs()[index++]->getName());
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
