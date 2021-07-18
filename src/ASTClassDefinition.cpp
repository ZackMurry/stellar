//
// Created by zack on 7/15/21.
//

#include "include/ASTClassDefinition.h"

bool isArrayType(const string& s) {
    return s.at(s.length() - 2) == '[' && s.at(s.length() - 1) == ']';
}

llvm::Value* ASTClassDefinition::codegen(llvm::IRBuilder<> *builder,
                                         llvm::LLVMContext *context,
                                         llvm::BasicBlock *entryBlock,
                                         map<string, llvm::Value *> *namedValues,
                                         llvm::Module *module,
                                         map<string, string>* objectTypes,
                                         map<string, ClassData>* classes) {
    auto classType = llvm::StructType::create(*context, name);
    vector<llvm::Type*> fieldLLVMTypes;
    map<string, llvm::Type*> llvmFields;
    for (const auto& field : fields) {
        int variableType = getVariableTypeFromString(field.second);
        if (variableType != -1) {
            auto t = getLLVMTypeByVariableType((VariableType) variableType, context);
            fieldLLVMTypes.push_back(t);
            llvmFields.insert({field.first, t });
        } else if (classes->count(field.second)) {
            auto t = classes->at(field.second).type;
            fieldLLVMTypes.push_back(t);
            llvmFields.insert({field.first, t });
        } else if (isArrayType(field.second)) {
            cout << "Array field type" << endl;
            string elType = field.second.substr(0, field.second.length() - 2); // Remove []
            variableType = getVariableTypeFromString(elType);
            auto ptrType = llvm::PointerType::get(getLLVMTypeByVariableType((VariableType) variableType, context), 0);
            fieldLLVMTypes.push_back(ptrType);
            llvmFields.insert({ field.first, ptrType });
        } else {
            cerr << "Error: expected type for field type but instead found " << field.second;
            exit(EXIT_FAILURE);
        }
    }
    classType->setBody(fieldLLVMTypes);
    classes->insert({ name, {classType, llvmFields } });

    // Methods
    map<string, llvm::Function*> llvmMethods;
    for (const auto& method : methods) {
        // Prepend class name to function name
        method.second->setName(name + "__" + method.first);

        // Add "t" arg for reference to the class
        method.second->addArg(new ASTVariableDefinition("t", name));
        cout << "arg length: " << method.second->getArgs().size() << endl;

        auto m = (llvm::Function*) method.second->codegen(builder, context, entryBlock, namedValues, module, objectTypes, classes);
        llvmMethods.insert({ name, m });
    }
    classes->erase(name);
    classes->insert({ name, {classType, llvmFields, llvmMethods } });
    return nullptr;
}
