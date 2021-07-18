//
// Created by zack on 7/9/21.
//

#include <utility>

#include "parser.h"
#include "ASTVariableDefinition.h"

#ifndef STELLAR_ASTFUNCTIONDEFINITION_H
#define STELLAR_ASTFUNCTIONDEFINITION_H

using namespace std;

class ASTFunctionDefinition : public ASTNode {
    string name;
    vector<ASTVariableDefinition*> args;
    vector<ASTNode*> body;
    string returnType;
public:
    ASTFunctionDefinition(string name, vector<ASTVariableDefinition*> args, vector<ASTNode*> body, string returnType) : name(move(name)), args(move(args)), body(move(body)), returnType(move(returnType)) {}
    string toString() override {
        string s = "[FUN_DEF: " + name + " args: [";
        for (const auto& arg : args) {
            s += arg->toString();
        }
        s += "] body: [";
        for (const auto& line : body) {
            s += line->toString();
        }
        s += "] ]";
        return s;
    }
    llvm::Value *codegen(llvm::IRBuilder<>* builder,
                         llvm::LLVMContext* context,
                         llvm::BasicBlock* entryBlock,
                         std::map<std::string, llvm::Value*>* namedValues,
                         llvm::Module* module,
                         map<string, string>* objectTypes,
                         map<string, ClassData>* classes) override;
    void setName(const string& n) {
        this->name = n;
    }
    vector<ASTVariableDefinition*> getArgs() {
        return args;
    }
    void setArgs(vector<ASTVariableDefinition*> a) {
        this->args = move(a);
    }
    void addArg(ASTVariableDefinition* arg) {
        // todo: it would be nicer to put it at the front
        this->args.push_back(arg);
    }
};


#endif //STELLAR_ASTFUNCTIONDEFINITION_H
