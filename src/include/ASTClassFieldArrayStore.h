//
// Created by zack on 7/18/21.
//

#include "parser.h"
#ifndef STELLAR_ASTCLASSFIELDARRAYSTORE_H
#define STELLAR_ASTCLASSFIELDARRAYSTORE_H


class ASTClassFieldArrayStore : public ASTNode {
    string identifier;
    string fieldName;
    ASTNode* index;
    ASTNode* value;
public:
    ASTClassFieldArrayStore(string identifier, string fieldName, ASTNode* index, ASTNode* value) : identifier(move(identifier)), fieldName(move(fieldName)), index(index), value(value) {}
    string toString() override {
        return "[CLASS_FIELD_ARR_STO: " + identifier + "." + fieldName + " at " + index->toString() + " to " + value->toString() + "]";
    }
    llvm::Value* codegen(llvm::IRBuilder<>* builder,
                         llvm::LLVMContext* context,
                         llvm::BasicBlock* entryBlock,
                         map<string, llvm::Value*>* namedValues,
                         llvm::Module* module,
                         map<string, string>* objectTypes,
                         map<string, ClassData>* classes) override;
};


#endif //STELLAR_ASTCLASSFIELDARRAYSTORE_H
