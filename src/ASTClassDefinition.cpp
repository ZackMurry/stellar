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
            auto t = classes->at(field.type).type;
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
