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
    map<string, llvm::Type*> LLVMFields;
    for (const auto& field : fields) {
        int variableType = getVariableTypeFromString(field.second);
        if (variableType != -1) {
            auto t = getLLVMTypeByVariableType((VariableType) variableType, context);
            fieldLLVMTypes.push_back(t);
            LLVMFields.insert({ field.first, t });
        } else if (classes->count(field.second)) {
            auto t = classes->at(field.second).type;
            fieldLLVMTypes.push_back(t);
            LLVMFields.insert({ field.first, t });
        } else {
            cerr << "Error: expected type for field type but instead found " << field.second;
            exit(EXIT_FAILURE);
        }
    }
    classType->setBody(fieldLLVMTypes);
    classes->insert({ name, { classType, LLVMFields } });
    return nullptr;
}
