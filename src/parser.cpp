//
// Created by zack on 7/1/21.
//

#include <utility>
#include <vector>
#include <iostream>
#include <map>
#include <string>
#include "include/parser.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/Host.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/TargetRegistry.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/Target/TargetOptions.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/IR/Verifier.h"

using namespace std;

unsigned long parsingIndex = 0;

static llvm::LLVMContext* context;
static llvm::IRBuilder<>* builder;
static llvm::Module* module;
static map<string, llvm::Value*> namedValues;

void printOutOfTokensError() {
    cerr << "Error: expected token, but nothing found at token " << parsingIndex << endl;
    exit(EXIT_FAILURE);
}

enum ExpressionOperator {
    OPERATOR_PLUS,
    OPERATOR_MINUS,
    OPERATOR_TIMES,
    OPERATOR_DIVIDE,
    OPERATOR_LT,
    OPERATOR_GT,
    OPERATOR_LE,
    OPERATOR_GE,
    OPERATOR_EQ,
    OPERATOR_NE
};

class ASTBinaryExpression : public ASTNode {
    ExpressionOperator op;
    ASTNode *lhs, *rhs;
public:
    ASTBinaryExpression(ExpressionOperator op, ASTNode* lhs, ASTNode* rhs) : op(op), lhs(lhs), rhs(rhs) {}
    string toString() override {
        return "[BIN_EXPRESSION: " + to_string(op) + " " + lhs->toString() + " " + rhs->toString() + "]";
    }
    llvm::Value *codegen() override;
};

llvm::Value* ASTBinaryExpression::codegen() {
    llvm::Value* l = lhs->codegen();
    llvm::Value* r = rhs->codegen();
    switch (op) {
        case OPERATOR_PLUS:
            return builder->CreateAdd(l, r, "addtmp");
        case OPERATOR_MINUS:
            return builder->CreateSub(l, r, "subtmp");
        case OPERATOR_TIMES:
            return builder->CreateMul(l, r, "multmp");
        case OPERATOR_DIVIDE:
            cerr << "Parser: division is not implemented" << endl;
            exit(EXIT_FAILURE);
        case OPERATOR_LT:
            if (l->getType()->isIntegerTy()) {
                return builder->CreateICmpSLT(l, r, "cmptmp");
            } else if (l->getType()->isFloatingPointTy()) {
                return builder->CreateFCmpOLT(l, r, "cmptmp");
            } else {
                cerr << "Error: unimplemented < operator" << endl;
                exit(EXIT_FAILURE);
            }
        case OPERATOR_GT:
            if (l->getType()->isIntegerTy()) {
                return builder->CreateICmpSGT(l, r, "cmptmp");
            } else if (l->getType()->isFloatingPointTy()) {
                return builder->CreateFCmpOGT(l, r, "cmptmp");
            } else {
                cerr << "Error: unimplemented > operator" << endl;
                exit(EXIT_FAILURE);
            }
        case OPERATOR_EQ:
            if (l->getType()->isIntegerTy()) {
                return builder->CreateICmpEQ(l, r, "cmptmp");
            } else if (l->getType()->isFloatingPointTy()) {
                return builder->CreateFCmpOEQ(l, r, "cmptmp");
            } else {
                cerr << "Error: unimplemented > operator" << endl;
                exit(EXIT_FAILURE);
            }
        case OPERATOR_LE:
            if (l->getType()->isIntegerTy()) {
                return builder->CreateICmpSLE(l, r, "cmptmp");
            } else if (l->getType()->isFloatingPointTy()) {
                return builder->CreateFCmpOLE(l, r, "cmptmp");
            } else {
                cerr << "Error: unimplemented > operator" << endl;
                exit(EXIT_FAILURE);
            }
        case OPERATOR_GE:
            if (l->getType()->isIntegerTy()) {
                return builder->CreateICmpSGE(l, r, "cmptmp");
            } else if (l->getType()->isFloatingPointTy()) {
                return builder->CreateFCmpOGE(l, r, "cmptmp");
            } else {
                cerr << "Error: unimplemented > operator" << endl;
                exit(EXIT_FAILURE);
            }
        case OPERATOR_NE:
            if (l->getType()->isIntegerTy()) {
                return builder->CreateICmpNE(l, r, "cmptmp");
            } else if (l->getType()->isFloatingPointTy()) {
                return builder->CreateFCmpONE(l, r, "cmptmp");
            } else {
                cerr << "Error: unimplemented > operator" << endl;
                exit(EXIT_FAILURE);
            }
        default:
            cerr << "Error: unimplemented binary expression" << endl;
            exit(EXIT_FAILURE);
    }
}

enum VariableType {
    VARIABLE_TYPE_I8,
    VARIABLE_TYPE_I16,
    VARIABLE_TYPE_I32,
    VARIABLE_TYPE_I64,
    VARIABLE_TYPE_F,
    VARIABLE_TYPE_D,
    VARIABLE_TYPE_B,
    VARIABLE_TYPE_V,
};

class ASTVariableDefinition : public ASTNode {
    string name;
    VariableType type;
public:
    ASTVariableDefinition(string name, VariableType type) : name(move(name)), type(type) {}
    string toString() override {
        return "[VAR_DEF: " + name + " " + to_string(type) + "]";
    }
    llvm::Value *codegen() override;
    VariableType getType() { return type; };
    string getName() { return name; };
};

static llvm::BasicBlock* entryBlock;
static llvm::BasicBlock* currBlock;

void createEntryFunction() {
    llvm::FunctionType* ft = llvm::FunctionType::get(llvm::Type::getVoidTy(*context), false);
    llvm::Function* func = llvm::Function::Create(ft, llvm::Function::ExternalLinkage, "STELLAR_ENTRY", *module);
    entryBlock = llvm::BasicBlock::Create(*context, "entry", func);
    currBlock = entryBlock;
}

llvm::Type* getLLVMTypeByVariableType(VariableType type) {
    if (type == VARIABLE_TYPE_F) {
        return llvm::Type::getFloatTy(*context);
    } else if (type == VARIABLE_TYPE_D) {
        return llvm::Type::getDoubleTy(*context);
    } else if (type == VARIABLE_TYPE_I32) {
        return llvm::Type::getInt32Ty(*context);
    } else if (type == VARIABLE_TYPE_V) {
        return llvm::Type::getVoidTy(*context);
    } else if (type == VARIABLE_TYPE_I8) {
        return llvm::Type::getInt8Ty(*context);
    } else if (type == VARIABLE_TYPE_I16) {
        return llvm::Type::getInt16Ty(*context);
    } else if (type == VARIABLE_TYPE_I64) {
        return llvm::Type::getInt64Ty(*context);
    } else {
        cerr << "Parser: unimplemented type " << type << endl;
        exit(EXIT_FAILURE);
    }
}

llvm::Value* ASTVariableDefinition::codegen() {
    llvm::Type* llvmType = getLLVMTypeByVariableType(type);
    builder->SetInsertPoint(currBlock);
    llvm::AllocaInst* alloca = builder->CreateAlloca(llvmType, nullptr, name);
    namedValues[name] = alloca;
    return alloca;
}

class ASTVariableDeclaration : public ASTNode {
    string name;
    VariableType type;
    ASTNode* value;
public:
    ASTVariableDeclaration(string name, VariableType type, ASTNode* value) : name(move(name)), type(type), value(value) {}
    string toString() override {
        return "[VAR_DECL: " + name + " " + to_string(type) + " " + value->toString() + "]";
    }
    llvm::Value *codegen() override;
};

llvm::Value* ASTVariableDeclaration::codegen() {
    llvm::Type* llvmType = getLLVMTypeByVariableType(type);
    builder->SetInsertPoint(currBlock);
    llvm::AllocaInst* alloca = builder->CreateAlloca(llvmType, nullptr, name);
    builder->CreateStore(value->codegen(), alloca);
    namedValues[name] = alloca;
    return alloca;
}

class ASTVariableAssignment : public ASTNode {
    string name;
    ASTNode* value;
public:
    ASTVariableAssignment(string name, ASTNode* value) : name(move(name)), value(value) {};
    string toString() override {
        return "[VAR_ASSIGN: " + name + " " + value->toString() + "]";
    }
    llvm::Value* codegen() override;
};

llvm::Value* ASTVariableAssignment::codegen() {
    cout << "Getting named val" << endl;
    cout << "named val: " << namedValues[name]->getName().str() << endl;
    llvm::Value* var = namedValues[name];
    if (!var) { // todo: this can't detect use of variables declared in a function outside of the function
        cerr << "Error: illegal use of undeclared variable '" << name << "'" << endl;
        exit(EXIT_FAILURE);
    }
    builder->CreateStore(value->codegen(), var);
    return var;
}

class ASTVariableExpression : public ASTNode {
    string name;
public:
    explicit ASTVariableExpression(string name) : name(move(name)) {}
    string toString() override {
        return "[VAR_EXP: " + name + "]";
    }
    llvm::Value *codegen() override;
};

llvm::Value* ASTVariableExpression::codegen() {
    llvm::Value* v = namedValues.at(name);
    if (!v) {
        cerr << "Parser: undeclared variable " << name << endl;
        exit(EXIT_FAILURE);
    }
    return builder->CreateLoad(v, name.c_str());
}

class ASTNumberExpression : public ASTNode {
    string val;
    VariableType type;
public:
    ASTNumberExpression(string val, VariableType type) : val(move(val)), type(type) {}
    string toString() override {
        return "[NUM_EXP: " + val + " type: " + to_string(type) + "]";
    }
    llvm::Value *codegen() override;
};

llvm::Value* ASTNumberExpression::codegen() {
    cout << "Number expression codegen" << endl;
    if (type == VARIABLE_TYPE_I32) {
        return llvm::ConstantInt::get(llvm::Type::getInt32Ty(*context), val, 10);
    } else if (type == VARIABLE_TYPE_F) {
        return llvm::ConstantFP::get(llvm::Type::getFloatTy(*context), val);
    } else if (type == VARIABLE_TYPE_D) {
        return llvm::ConstantFP::get(llvm::Type::getDoubleTy(*context), val);
    } else if (type == VARIABLE_TYPE_I8) {
        return llvm::ConstantInt::get(llvm::Type::getInt8Ty(*context), val, 10);
    } else if (type == VARIABLE_TYPE_I16) {
        cout << "i16 type" << endl;
        return llvm::ConstantInt::get(llvm::Type::getInt16Ty(*context), val, 10);
    } else if (type == VARIABLE_TYPE_I64) {
        return llvm::ConstantInt::get(llvm::Type::getInt64Ty(*context), val, 10);
    } else {
        cerr << "Error: unimplemented number type " << type << endl;
        exit(EXIT_FAILURE);
    }
}

class ASTFunctionDefinition : public ASTNode {
    string name;
    vector<ASTVariableDefinition*> args;
    vector<ASTNode*> body;
    VariableType returnType;
public:
    ASTFunctionDefinition(string name, vector<ASTVariableDefinition*> args, vector<ASTNode*> body, VariableType returnType) : name(move(name)), args(move(args)), body(move(body)), returnType(returnType) {}
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
    llvm::Value* codegen() override;
};

llvm::Value* ASTFunctionDefinition::codegen() {
    cout << "FuncDef codegen" << endl;
    llvm::Function* func = module->getFunction(name);
    if (func && !func->empty()) {
        cerr << "Error: function " << name << " cannot be redefined" << endl;
        exit(EXIT_FAILURE);
    }
    vector<llvm::Type*> argTypes;
    if (!func) {
        for (const auto& arg : args) {
            llvm::Type* llvmType = getLLVMTypeByVariableType(arg->getType());
            if (llvmType == nullptr) {
                cerr << "Error mapping variable type to LLVM type" << endl;
                exit(EXIT_FAILURE);
            }
            argTypes.push_back(llvmType);
        }
        llvm::FunctionType* ft = llvm::FunctionType::get(getLLVMTypeByVariableType(returnType), argTypes, false);
        func = llvm::Function::Create(ft, llvm::Function::ExternalLinkage, name, *module);
        unsigned index = 0;
        for (auto &arg : func->args()) {
            arg.setName(args[index++]->getName());
        }
    } else {
        // Get arg types
        for (const auto& arg : func->args()) {
            argTypes.push_back(arg.getType());
        }
    }
    llvm::BasicBlock* bb = llvm::BasicBlock::Create(*context, "entry", func);
    cout << "Created basic block" << endl;
    builder->SetInsertPoint(bb);
    namedValues.clear();
    unsigned index = 0;
    for (auto &arg : func->args()) {
        llvm::IRBuilder<> tempBuilder(&func->getEntryBlock(), func->getEntryBlock().begin());
        llvm::AllocaInst* alloca = tempBuilder.CreateAlloca(argTypes[index++], nullptr, arg.getName());
        builder->CreateStore(&arg, alloca);
        namedValues[string(arg.getName())] = alloca;
    }
    currBlock = bb;
    for (auto &node : body) {
        node->codegen();
    }
    cout << "For loops done" << endl;
    // If the final block is empty, add a return statement to it so that it is not empty
    if (currBlock->empty()) {
        builder->CreateRetVoid();
    }
    currBlock = entryBlock;
    builder->SetInsertPoint(currBlock);
    llvm::verifyFunction(*func);
    return func;
}

class ASTFunctionInvocation : public ASTNode {
    string name;
    vector<ASTNode*> args;
public:
    ASTFunctionInvocation(string name, vector<ASTNode*> args) : name(move(name)), args(move(args)) {}
    string toString() override {
        string s = "[FUN_INV: " + name + " args: [";
        for (auto const& arg : args) {
            s += arg->toString();
        }
        s += "]]";
        return s;
    }
    llvm::Value* codegen() override;
};

llvm::Value* ASTFunctionInvocation::codegen() {
    llvm::Function* calleeFunc = module->getFunction(name);
    if (!calleeFunc) {
        cerr << "Error: unknown function reference" << endl;
        exit(EXIT_FAILURE);
    }
    if (calleeFunc->arg_size() != args.size()) {
        cerr << "Error: incorrect number of arguments passed to function " << name << endl;
        exit(EXIT_FAILURE);
    }
    vector<llvm::Value*> argsV;
    for (auto & arg : args) {
        argsV.push_back(arg->codegen());
    }
    builder->SetInsertPoint(currBlock);
    return builder->CreateCall(calleeFunc, argsV, "calltmp");
}

class ASTArrayDefinition : public ASTNode {
    string name;
    VariableType elementType;
    ASTNode* length;
public:
    ASTArrayDefinition(string name, VariableType elementType, ASTNode* length) : name(move(name)), elementType(elementType), length(length) {}
    string toString() override {
        return "[ARR_DEF: " + name + " " + to_string(elementType) + " size: " + length->toString() + "]";
    }
    llvm::Value* codegen() override;
};

llvm::Value* ASTArrayDefinition::codegen() {
    llvm::Type* llvmElType = getLLVMTypeByVariableType(elementType);
    builder->SetInsertPoint(currBlock);
    llvm::AllocaInst* alloca = builder->CreateAlloca(llvmElType, length->codegen(), name);
    namedValues[name] = alloca;
    return alloca;
}

class ASTArrayAccess : public ASTNode {
    string name;
    ASTNode* index;
public:
    ASTArrayAccess(string name, ASTNode* index) : name(move(name)), index(index) {}
    string toString() override {
        return "[ARR_ACCESS: " + name + " at " + index->toString() + "]";
    }
    llvm::Value* codegen() override;
};

llvm::Type* getLLVMPtrTypeByType(llvm::Type* type) {
    if (type->isIntegerTy()) {
        if (type->isIntegerTy(8)) {
            cout << "type: i8" << endl;
            return llvm::Type::getInt8PtrTy(*context);
        }
        if (type->isIntegerTy(16)) {
            cout << "type: i16" << endl;
            return llvm::Type::getInt16PtrTy(*context);
        }
        if (type->isIntegerTy(32)) {
            cout << "type: i32" << endl;
            return llvm::Type::getInt32PtrTy(*context);
        }
        if (type->isIntegerTy(64)) {
            cout << "type: i64" << endl;
            return llvm::Type::getInt64PtrTy(*context);
        }
        cerr << "Error: unknown integer type" << endl;
        exit(EXIT_FAILURE);
    }
    if (type->isFloatingPointTy()) {
        if (type->isFloatTy()) {
            cout << "type: f" << endl;
            return llvm::Type::getFloatPtrTy(*context);
        }
        if (type->isDoubleTy()) {
            cout << "type; d" << endl;
            return llvm::Type::getDoublePtrTy(*context);
        }
    }
    cerr << "Error: unknown type" << endl;
    exit(EXIT_FAILURE);
}

llvm::Value* ASTArrayAccess::codegen() {
    builder->SetInsertPoint(currBlock);
    auto* gep = builder->CreateInBoundsGEP(namedValues[name], index->codegen(), "acctmp");
    gep->getType()->isIntegerTy(32);
    gep->mutateType(getLLVMPtrTypeByType(namedValues[name]->getType()->getPointerElementType()));
    return builder->CreateLoad(gep, "loadtmp");
}

class ASTArrayIndexAssignment : public ASTNode {
    string name;
    ASTNode* index;
    ASTNode* value;
public:
    ASTArrayIndexAssignment(string name, ASTNode* index, ASTNode* value) : name(move(name)), index(index), value(value) {}
    string toString() override {
        return "[ARR_ASSIGN: " + name + " at " + index->toString() + " to " + value->toString() + "]";
    }
    llvm::Value* codegen() override;
};

llvm::Value* ASTArrayIndexAssignment::codegen() {
    builder->SetInsertPoint(currBlock);
    llvm::Value* ref = builder->CreateInBoundsGEP(namedValues[name], llvm::ArrayRef<llvm::Value*>(index->codegen()), "acctmp");
    return builder->CreateStore(value->codegen(), ref);
}

// todo: logical and
class ASTIfStatement : public ASTNode {
    ASTBinaryExpression* condition;
    vector<ASTNode*> ifBody;
    vector<ASTNode*> elseBody;
public:
    ASTIfStatement(ASTBinaryExpression* condition, vector<ASTNode*> ifBody, vector<ASTNode*> elseBody) : condition(condition), ifBody(move(ifBody)), elseBody(move(elseBody)) {}
    string toString() override {
        string s = "[IF_STMT: condition: " + condition->toString() + " ifBody: ["; //+ ifBody->toString() + " elseBody: " + (elseBody ? elseBody ->toString() : "[none]") + "]";
        for (auto const& stmt : ifBody) {
            s += stmt->toString();
        }
        s += "] elseBody: [";
        if (!elseBody.empty()) {
            for (auto const& stmt : ifBody) {
                s += stmt->toString();
            }
        } else {
            s += "none";
        }
        s += "]]";
        return s;
    }
    llvm::Value* codegen() override;
};

llvm::Value* ASTIfStatement::codegen() {
    llvm::Value* conditionValue = condition->codegen();
    llvm::BasicBlock* ifBB = llvm::BasicBlock::Create(*context, "ifbody");
    llvm::BasicBlock* elseBB = llvm::BasicBlock::Create(*context, "elsebody");
    llvm::BasicBlock* mergeBB = llvm::BasicBlock::Create(*context, "mergeif");
    builder->CreateCondBr(conditionValue, ifBB, elseBB);
    currBlock->getParent()->getBasicBlockList().push_back(ifBB);
    currBlock->getParent()->getBasicBlockList().push_back(elseBB);
    currBlock->getParent()->getBasicBlockList().push_back(mergeBB);
    builder->SetInsertPoint(ifBB);
    auto oldBlock = currBlock;
    currBlock = ifBB;
    for (auto const& line : ifBody) {
        line->codegen();
    }
    builder->CreateBr(mergeBB);
    builder->SetInsertPoint(elseBB);
    currBlock = elseBB;
    for (auto const& line : elseBody) {
        line->codegen();
    }
    builder->CreateBr(mergeBB);
    currBlock = mergeBB;
    if (oldBlock == entryBlock) {
        entryBlock = mergeBB;
    }
    builder->SetInsertPoint(mergeBB);
    return mergeBB;
}

ASTNode* parseIdentifierExpression(vector<Token> tokens);

ASTNode* parseExpression(const vector<Token>& tokens);

ASTNode* parseNumberExpression(vector<Token> tokens) {
    string val = tokens[parsingIndex++].value;
    VariableType type = VARIABLE_TYPE_I32;
    int lastDigitIndex = -1;
    for (int i = 0; i < val.size(); i++) {
        cout << "i: " << i << endl;
        if (val.at(i) == '.') {
            if (type == VARIABLE_TYPE_F) {
                cerr << "Error: expected a maximum of one decimal to occur in a number" << endl;
                exit(EXIT_FAILURE);
            }
            type = VARIABLE_TYPE_F;
            lastDigitIndex = i;
        } else if (!isdigit(val.at(i))) {
            lastDigitIndex = i - 1;
            break;
        } else {
            lastDigitIndex = i;
        }
    }
    if (lastDigitIndex == -1) {
        cerr << "Error: expected number to start with at least one digit (number was " << val << ")" << endl;
        exit(EXIT_FAILURE);
    }
    string numericPart = val.substr(0, lastDigitIndex + 1);
    string typePart = val.substr(lastDigitIndex + 1);
    cout << "Numeric part: " << numericPart << "; type part: " << typePart << endl;
    if (!typePart.empty()) {
        if (typePart == "f") {
            cout << "Number is an explicit float type" << endl;
            type = VARIABLE_TYPE_F;
        } else if (typePart == "d") {
            cout << "Number is an explicit double type" << endl;
            type = VARIABLE_TYPE_D;
        } else if (typePart == "i32") {
            cout << "Number is an explicit i32 type" << endl;
            type = VARIABLE_TYPE_I32;
        } else if (typePart == "i64") {
            cout << "Number is an explicit i64 type" << endl;
            type = VARIABLE_TYPE_I64;
        } else if (typePart == "i16") {
            cout << "Number is an explicit i16 type" << endl;
            type = VARIABLE_TYPE_I16;
        } else if (typePart == "i8") {
            cout << "Number is an explicit i8 type" << endl;
            type = VARIABLE_TYPE_I8;
        } else {
            cerr << "Error: unknown explicit variable type suffix " << typePart << endl;
            exit(EXIT_FAILURE);
        }
    }
    return new ASTNumberExpression(numericPart, type);
}

ASTNode* parseParenExpression(vector<Token> tokens) {
    parsingIndex++; // Consume '('
    auto v = parseExpression(tokens);
    if (tokens[parsingIndex].type != TOKEN_PUNCTUATION || tokens[parsingIndex].value != ")") {
        cerr << "Error: expected ')'" << endl;
        exit(EXIT_FAILURE);
    }
    parsingIndex++; // Consume ')'
    return v;
}

ASTNode* parsePrimary(vector<Token> tokens) {
    switch (tokens[parsingIndex].type) {
        case TOKEN_IDENTIFIER:
            return parseIdentifierExpression(tokens);
        case TOKEN_NUMBER:
            return parseNumberExpression(tokens);
        case TOKEN_PUNCTUATION:
            if (tokens[parsingIndex].value == "(") {
                return parseParenExpression(tokens);
            } else {
                cerr << "Error: unknown token found when parsing expression (" << tokens[parsingIndex].value << ")" << endl;
                exit(EXIT_FAILURE);
            }
        default:
            cerr << "Error: unknown token found when parsing expression" << endl;
            exit(EXIT_FAILURE);
    }
}

int getTokenPrecedence(const Token& token) {
    if (token.type != TOKEN_PUNCTUATION) {
        return -1;
    }
    if (token.value == "+") {
        return 20;
    }
    if (token.value == "-") {
        return 20;
    }
    if (token.value == "*") {
        return 40;
    }
    if (token.value == "/") {
        return 40;
    }
    return -1;
}

ASTNode* parseBinOpRHS(vector<Token> tokens, int exprPrec, ASTNode* lhs) {
    cout << "Parsing binary operator RHS" << endl;
    while (true) {
        if (tokens[parsingIndex].type == TOKEN_EOF || tokens[parsingIndex].type == TOKEN_NEWLINE/* || (tokens[parsingIndex].type == TOKEN_PUNCTUATION && tokens[parsingIndex].value == "]")*/) {
            return lhs;
        }
        int tokenPrec = getTokenPrecedence(tokens[parsingIndex]);
        if (tokenPrec < exprPrec) {
            return lhs;
        }
        ExpressionOperator binOp;
        if (tokens[parsingIndex].value == "+") {
            binOp = OPERATOR_PLUS;
        } else if (tokens[parsingIndex].value == "-") {
            binOp = OPERATOR_MINUS;
        } else if (tokens[parsingIndex].value == "*") {
            binOp = OPERATOR_TIMES;
        } else if (tokens[parsingIndex].value == "/") {
            binOp = OPERATOR_DIVIDE;
        } else {
            cerr << "Error: unknown binary operator (" << tokens[parsingIndex].value << ")" << endl;
            exit(EXIT_FAILURE);
        }
        parsingIndex++;
        auto rhs = parsePrimary(tokens);
        int nextPrec = getTokenPrecedence(tokens[parsingIndex]);
        if (tokenPrec < nextPrec) {
            rhs = parseBinOpRHS(tokens, tokenPrec + 1, rhs);
        }
        lhs = new ASTBinaryExpression(binOp, lhs, rhs);
    }
}

ASTNode* parseExpression(const vector<Token>& tokens) {
    auto lhs = parsePrimary(tokens);
    if (tokens[parsingIndex].type == TOKEN_NEWLINE || tokens[parsingIndex].type == TOKEN_EOF) {
        return lhs;
    }
    return parseBinOpRHS(tokens, 0, lhs);
}

int getVariableTypeFromToken(const Token& token) {
    if (token.type != TOKEN_IDENTIFIER) {
        cout << "Not an identifier" << endl;
        return -1;
    }
    if (token.value == "i32") {
        return VARIABLE_TYPE_I32;
    } else if (token.value == "f") {
        return VARIABLE_TYPE_F;
    } else if (token.value == "d") {
        return VARIABLE_TYPE_D;
    } else if (token.value == "b") {
        return VARIABLE_TYPE_B;
    } else if (token.value == "i8") {
        return VARIABLE_TYPE_I8;
    } else if (token.value == "i16") {
        return VARIABLE_TYPE_I16;
    } else if (token.value == "i64") {
        return VARIABLE_TYPE_I64;
    } else if (token.value == "v") {
        return VARIABLE_TYPE_V;
    } else {
        return -1;
    }
}

vector<ASTNode*> parseWithoutTokenizing(vector<Token> tokens);

// Expects parsingIndex to be after '{'. Consumes ending brace
vector<ASTNode*> getBodyOfBlock(vector<Token> tokens) {
    cout << "Getting body of block" << endl;
    int stackSize = 0;
    unsigned long end = parsingIndex;
    vector<Token> bodyTokens;
    while (stackSize > 0 || (tokens[end].type != TOKEN_PUNCTUATION || tokens[end].value != "}")) {
        if (end + 1 >= tokens.size()) {
            cout << "Out of tokens in block" << endl;
            printOutOfTokensError();
        }
        if (tokens[end].type == TOKEN_PUNCTUATION && tokens[end].value == "{") {
            stackSize++;
        } else if (tokens[end].type == TOKEN_PUNCTUATION && tokens[end].value == "}") {
            stackSize--;
        }
        end++;
        if (stackSize == 0 && tokens[end].type == TOKEN_PUNCTUATION && tokens[end].value == "}") {
            break;
        }
        bodyTokens.push_back(tokens[end]);
    }
    cout << "Body token size: " << bodyTokens.size() << endl;
    vector<ASTNode*> body = parseWithoutTokenizing(bodyTokens);
    parsingIndex = end + 1;
    return body;
}

// Expects the parsingIndex to be at an opening parenthesis
ASTNode* parseFunctionDefinition(vector<Token> tokens, VariableType type, const string& name) {
    cout << "Function definition" << endl;
    vector<ASTVariableDefinition*> args;
    while (++parsingIndex < tokens.size()) {
        if (tokens[parsingIndex].type == TOKEN_PUNCTUATION && tokens[parsingIndex].value == ")") {
            break;
        }
        if (tokens[parsingIndex].type != TOKEN_IDENTIFIER) {
            cerr << "Error: expected type in function parameter" << endl;
            exit(EXIT_FAILURE);
        }
        int ivt = getVariableTypeFromToken(tokens[parsingIndex]);
        if (ivt == -1) {
            cerr << "Error: expected type in function parameter" << endl;
            exit(EXIT_FAILURE);
        }
        parsingIndex++;
        if (tokens[parsingIndex].type != TOKEN_IDENTIFIER) {
            cerr << "Error: expected parameter name after type" << endl;
            exit(EXIT_FAILURE);
        }
        args.push_back(new ASTVariableDefinition(tokens[parsingIndex].value, (VariableType) ivt));
        cout << "arg: " << args.back()->getName() << endl;
        if (++parsingIndex >= tokens.size()) {
            printOutOfTokensError();
        }
        if (tokens[parsingIndex].type != TOKEN_PUNCTUATION || tokens[parsingIndex].value != ",") {
            break;
        }
    }
    if (tokens[parsingIndex].type != TOKEN_PUNCTUATION || tokens[parsingIndex].value != ")") {
        cerr << "Error: expected ')' after function parameters" << endl;
        exit(EXIT_FAILURE);
    }
    // Consume ')'
    if (++parsingIndex >= tokens.size()) {
        printOutOfTokensError();
    }
    if (tokens[parsingIndex].type != TOKEN_PUNCTUATION || tokens[parsingIndex].value != "{") {
        cerr << "Error: expected '{' after function prototype" << endl;
        exit(EXIT_FAILURE);
    }
    // Consume '{'
    if (++parsingIndex >= tokens.size()) {
        printOutOfTokensError();
    }
    vector<ASTNode*> body = getBodyOfBlock(tokens);
    cout << "body size: " << body.size() << endl;
    cout << "Arg size before creating: " << args.size() << endl;
    return new ASTFunctionDefinition(name, args, body, type);
}

// Expects parsingIndex to be at [
ASTNode* parseArrayDefinition(vector<Token> tokens, VariableType type) {
    // Consume '['
    if (++parsingIndex >= tokens.size()) {
        printOutOfTokensError();
    }
    ASTNode* length = parseExpression(tokens);
    cout << "Length: " << length->toString() << endl;
    if (tokens[parsingIndex].type != TOKEN_PUNCTUATION || tokens[parsingIndex].value != "]") {
        cerr << "Error: expected ending brace after array length" << endl;
        exit(EXIT_FAILURE);
    }
    // Consume ']'
    if (++parsingIndex >= tokens.size()) {
        printOutOfTokensError();
    }
    if (tokens[parsingIndex].type != TOKEN_IDENTIFIER) {
        cerr << "Error: expected identifier after array type" << endl;
        exit(EXIT_FAILURE);
    }
    string name = tokens[parsingIndex++].value; // Consume name
    return new ASTArrayDefinition(name, type, length);
}

ASTNode* parseVariableDeclaration(vector<Token> tokens, VariableType type) {
    cout << "Variable declaration: " << parsingIndex << endl;
    if (tokens[parsingIndex].type == TOKEN_PUNCTUATION && tokens[parsingIndex].value == "[") {
        cout << "Array definition" << endl;
        return parseArrayDefinition(tokens, type);
    }
    if (tokens[parsingIndex].type != TOKEN_IDENTIFIER) {
        cerr << "Error: expected identifier after variable type but found token type " << tokens[parsingIndex].type << ":" << tokens[parsingIndex].value << endl;
        exit(EXIT_FAILURE);
    }
    string name = tokens[parsingIndex++].value; // Get var name and consume token
    if (tokens[parsingIndex].type == TOKEN_PUNCTUATION && tokens[parsingIndex].value == "(") {
        cout << "Function definition" << endl;
        // todo returning arrays from functions
        return parseFunctionDefinition(tokens, type, name);
    } else if (type == VARIABLE_TYPE_V) {
        cerr << "Error: only functions can use the 'v' type" << endl;
        exit(EXIT_FAILURE);
    }
    if (tokens[parsingIndex].type != TOKEN_PUNCTUATION || tokens[parsingIndex].value != "=") {
        if (tokens[parsingIndex].type == TOKEN_NEWLINE) {
            parsingIndex++;
            return new ASTVariableDefinition(move(name), type);
        } else {
            cerr << "Error: expected new line or equals sign after variable definition but instead found token type " << tokens[parsingIndex].type << ":" << tokens[parsingIndex].value << endl;
            exit(EXIT_FAILURE);
        }
    }
    parsingIndex++; // Consume '='
    return new ASTVariableDeclaration(name, type, parseExpression(tokens));
}

// Expects current token to be the equals sign in 'a = E'
ASTNode* parseVariableAssignment(vector<Token> tokens, string name) {
    if (tokens[parsingIndex].type != TOKEN_PUNCTUATION || tokens[parsingIndex].value != "=") {
        cerr << "Unexpected parsing error: variable assignment parsing failed (= expected)" << endl;
        exit(EXIT_FAILURE);
    }
    parsingIndex++; // Consume '='
    return new ASTVariableAssignment(move(name), parseExpression(tokens));
}

// Expects parsingIndex to be at '['
ASTNode* parseArrayAccess(vector<Token> tokens, string name) {
    cout << "Array access" << endl;
    // Consume '['
    if (++parsingIndex >= tokens.size()) {
        printOutOfTokensError();
    }
    ASTNode* index = parseExpression(tokens);
    if (tokens[parsingIndex].type != TOKEN_PUNCTUATION || tokens[parsingIndex].value != "]") {
        cerr << "Error: expected ']' after array index" << endl;
        exit(EXIT_FAILURE);
    }
    if (++parsingIndex >= tokens.size()) {
        printOutOfTokensError();
    }
    if (tokens[parsingIndex].type == TOKEN_PUNCTUATION && tokens[parsingIndex].value == "=") {
        // Consume '='
        if (++parsingIndex >= tokens.size()) {
            printOutOfTokensError();
        }
        return new ASTArrayIndexAssignment(move(name), index, parseExpression(tokens));
    }
    return new ASTArrayAccess(move(name), index);
}

ASTNode* parseIdentifierExpression(vector<Token> tokens) {
    string identifier = tokens[parsingIndex].value; // Get and consume identifier
    int variableType = getVariableTypeFromToken(tokens[parsingIndex]);
    if (++parsingIndex >= tokens.size()) {
        printOutOfTokensError();
    }
    if (variableType != -1) {
        return parseVariableDeclaration(tokens, (VariableType) variableType);
    }
    if (tokens[parsingIndex].type == TOKEN_PUNCTUATION && tokens[parsingIndex].value == "=") {
        return parseVariableAssignment(tokens, identifier);
    }
    if (tokens[parsingIndex].type != TOKEN_PUNCTUATION || tokens[parsingIndex].value != "(") {
        cout << "Variable expression" << endl;
        if (tokens[parsingIndex].type == TOKEN_PUNCTUATION && tokens[parsingIndex].value == "[") {
            return parseArrayAccess(tokens, identifier);
        }
        return new ASTVariableExpression(identifier);
    }
    cout << "Function invocation" << endl;
    // Consume '('
    if (++parsingIndex >= tokens.size()) {
        printOutOfTokensError();
    }

    vector<ASTNode*> args;
    if (tokens[parsingIndex].type != TOKEN_PUNCTUATION || tokens[parsingIndex].value != ")") {
        while (true) {
            args.push_back(parseExpression(tokens));
            if (tokens[parsingIndex].type == TOKEN_PUNCTUATION && tokens[parsingIndex].value == ")") {
                break;
            }
            if (tokens[parsingIndex].type != TOKEN_PUNCTUATION || tokens[parsingIndex].value != ",") {
                cout << "i: " << parsingIndex << endl;
                cerr << "Error: expected ',' or ')' in argument list" << endl;
                exit(EXIT_FAILURE);
            }
            // Consume ','
            if (++parsingIndex >= tokens.size()) {
                printOutOfTokensError();
            }
        }
    }
    if (++parsingIndex >= tokens.size()) {
        printOutOfTokensError();
    }
    return new ASTFunctionInvocation(identifier, args);
}

class ASTReturn : public ASTNode {
    ASTNode* exp;
public:
    explicit ASTReturn(ASTNode* exp) : exp(exp) {}
    string toString() override {
        if (!exp) {
            return "[RETURN: void]";
        }
        return "[RETURN: " + exp->toString() + "]";
    }
    llvm::Value* codegen() override;
};

llvm::Value* ASTReturn::codegen() {
    if (exp) {
        llvm::Value* val = exp->codegen();
        return builder->CreateRet(val);
    } else { // Void
        return builder->CreateRetVoid();
    }
}

ASTNode* parseReturnExpression(vector<Token> tokens) {
    // Consume 'r'
    if (++parsingIndex >= tokens.size()) {
        printOutOfTokensError();
    }
    if (tokens[parsingIndex].type == TOKEN_NEWLINE) {
        return new ASTReturn(nullptr);
    }
    return new ASTReturn(parseExpression(tokens));
}

class ASTExternDeclaration : public ASTNode {
    string name;
    vector<ASTVariableDefinition*> args;
    VariableType returnType;
public:
    ASTExternDeclaration(string name, vector<ASTVariableDefinition*> args, VariableType returnType) : name(move(name)), args(move(args)), returnType(returnType) {}
    string toString() override {
        string s = "[EXTERN: " + to_string(returnType) + " " + name + " args: [";
        for (const auto& arg : args) {
            s += arg->toString();
        }
        s += "]]";
        return s;
    }
    llvm::Value* codegen() override;
};

llvm::Value* ASTExternDeclaration::codegen() {
    vector<llvm::Type*> argTypes;
    for (const auto& arg : args) {
        llvm::Type* llvmType = getLLVMTypeByVariableType(arg->getType());
        if (llvmType == nullptr) {
            cerr << "Error mapping variable type to LLVM type" << endl;
            exit(EXIT_FAILURE);
        }
        argTypes.push_back(llvmType);
    }
    llvm::FunctionType* ft = llvm::FunctionType::get(getLLVMTypeByVariableType(returnType), argTypes, false);
    llvm::Function* func = llvm::Function::Create(ft, llvm::Function::ExternalLinkage, name, *module);
    unsigned index = 0;
    for (auto &arg : func->args()) {
        arg.setName(args[index++]->getName());
    }
    return func;
}

ASTNode* parseExternExpression(vector<Token> tokens) {
    cout << "Extern definition" << endl;
    // Consume 'x'
    if (++parsingIndex >= tokens.size()) {
        printOutOfTokensError();
    }
    if (tokens[parsingIndex].type != TOKEN_IDENTIFIER) {
        cerr << "Error: expected function return type after 'x'" << endl;
        exit(EXIT_FAILURE);
    }
    int returnType = getVariableTypeFromToken(tokens[parsingIndex]);
    if (returnType == -1) {
        cerr << "Error: expected function return type after 'x' but instead found " << tokens[parsingIndex].value << endl;
        exit(EXIT_FAILURE);
    }
    if (++parsingIndex >= tokens.size()) {
        printOutOfTokensError();
    }
    if (tokens[parsingIndex].type != TOKEN_IDENTIFIER) {
        cerr << "Error: expected extern function's name after its return type" << endl;
        exit(EXIT_FAILURE);
    }
    string name = tokens[parsingIndex].value;
    // Consume name
    if (++parsingIndex >= tokens.size()) {
        printOutOfTokensError();
    }
    vector<ASTVariableDefinition*> args;
    while (++parsingIndex < tokens.size()) {
        if (tokens[parsingIndex].type == TOKEN_PUNCTUATION && tokens[parsingIndex].value == ")") {
            break;
        }
        if (tokens[parsingIndex].type != TOKEN_IDENTIFIER) {
            cerr << "Error: expected type in function parameter" << endl;
            exit(EXIT_FAILURE);
        }
        int ivt = getVariableTypeFromToken(tokens[parsingIndex]);
        if (ivt == -1 || ivt == VARIABLE_TYPE_V) {
            cerr << "Error: expected type in function parameter" << endl;
            exit(EXIT_FAILURE);
        }
        parsingIndex++;
        if (tokens[parsingIndex].type != TOKEN_IDENTIFIER) {
            cerr << "Error: expected parameter name after type" << endl;
            exit(EXIT_FAILURE);
        }
        args.push_back(new ASTVariableDefinition(tokens[parsingIndex].value, (VariableType) ivt));
        if (++parsingIndex >= tokens.size()) {
            printOutOfTokensError();
        }
        if (tokens[parsingIndex].type != TOKEN_PUNCTUATION || tokens[parsingIndex].value != ",") {
            break;
        }
    }
    if (tokens[parsingIndex].type != TOKEN_PUNCTUATION || tokens[parsingIndex].value != ")") {
        cerr << "Error: expected ')' after function parameters" << endl;
        exit(EXIT_FAILURE);
    }
    // Consume ')'
    parsingIndex++;
    return new ASTExternDeclaration(name, args, (VariableType) returnType);
}

// Expects parsing index to be at "if"
ASTNode* parseIfExpression(vector<Token> tokens) {
    cout << "If expression" << endl;
    // Consume "if"
    if (++parsingIndex >= tokens.size()) {
        printOutOfTokensError();
    }
    if (tokens[parsingIndex].type != TOKEN_PUNCTUATION || tokens[parsingIndex].value != "(") {
        cerr << "Error: expected '(' after if" << endl;
        exit(EXIT_FAILURE);
    }
    // Consume "("
    if (++parsingIndex >= tokens.size()) {
        printOutOfTokensError();
    }
    auto conditionLHS = parseExpression(tokens);

    if (tokens[parsingIndex].type != TOKEN_PUNCTUATION) {
        cerr << "Error: expected comparison operator after expression in if condition" << endl;
        exit(EXIT_FAILURE);
    }

    ExpressionOperator op;
    if (tokens[parsingIndex].value == "<") {
        op = OPERATOR_LT;
    } else if (tokens[parsingIndex].value == ">") {
        op = OPERATOR_GT;
    } else if (tokens[parsingIndex].value == "==") {
        op = OPERATOR_EQ;
    } else if (tokens[parsingIndex].value == "<=") {
        op = OPERATOR_LE;
    } else if (tokens[parsingIndex].value == ">=") {
        op = OPERATOR_GE;
    } else if (tokens[parsingIndex].value == "!=") {
        op = OPERATOR_NE;
    } else {
        cerr << "Error: expected comparison operator after expression in if condition" << endl;
        exit(EXIT_FAILURE);
    }
    // Consume comparison
    if (++parsingIndex >= tokens.size()) {
        printOutOfTokensError();
    }
    auto conditionRHS = parseExpression(tokens);

    auto* condition = new ASTBinaryExpression(op, conditionLHS, conditionRHS);
    cout << "Condition: " << condition->toString() << endl;

    if (tokens[parsingIndex].type != TOKEN_PUNCTUATION || tokens[parsingIndex].value != ")") {
        cerr << "Error: expected ')' after if condition" << endl;
        exit(EXIT_FAILURE);
    }
    // Consume ")"
    if (++parsingIndex >= tokens.size()) {
        printOutOfTokensError();
    }

    if (tokens[parsingIndex].type != TOKEN_PUNCTUATION || tokens[parsingIndex].value != "{") {
        cerr << "Error: expected '{' after if condition (single-line if statements without braces are not allowed)!" << endl;
        exit(EXIT_FAILURE);
    }
    // Consume "{"
    if (++parsingIndex >= tokens.size()) {
        printOutOfTokensError();
    }
    vector<ASTNode*> body = getBodyOfBlock(tokens);

    if (parsingIndex >= tokens.size() || tokens[parsingIndex].type != TOKEN_ELSE) {
        // Empty else body
        return new ASTIfStatement(condition, body, vector<ASTNode*>());
    }
    cout << "Parsing else" << endl;
    // Consume "else"
    if (++parsingIndex >= tokens.size()) {
        printOutOfTokensError();
    }
    // todo else if
    if (tokens[parsingIndex].type != TOKEN_PUNCTUATION || tokens[parsingIndex].value != "{") {
        cerr << "Error: expected '{' after else (single-line else blocks without braces are not allowed)!" << endl;
        exit(EXIT_FAILURE);
    }
    // Consume "{"
    if (++parsingIndex >= tokens.size()) {
        printOutOfTokensError();
    }
    cout << "parsing index when getting else body: " << parsingIndex << endl;
    vector<ASTNode*> elseBody = getBodyOfBlock(tokens);
    cout << "Else body: " << elseBody[0]->toString() << endl;
    cout << "Made it to the end of if parsing" << endl;
    return new ASTIfStatement(condition, body, elseBody);
}

void initializeModule() {
    context = new llvm::LLVMContext();
    module = new llvm::Module("module", *context);
    builder = new llvm::IRBuilder<>(*context);
    createEntryFunction();
}

vector<ASTNode*> parseWithoutTokenizing(vector<Token> tokens) {
    parsingIndex = 0;
    vector<ASTNode*> nodes;
    while (parsingIndex < tokens.size()) {
        if (tokens[parsingIndex].type == TOKEN_IDENTIFIER) {
            nodes.push_back(parseIdentifierExpression(tokens));
        } else if (tokens[parsingIndex].type == TOKEN_EOF || tokens[parsingIndex].type == TOKEN_NEWLINE) {
            parsingIndex++;
        } else if (tokens[parsingIndex].type == TOKEN_RETURN) {
            nodes.push_back(parseReturnExpression(tokens));
        } else if (tokens[parsingIndex].type == TOKEN_EXTERN) {
            nodes.push_back(parseExternExpression(tokens));
        } else if (tokens[parsingIndex].type == TOKEN_IF) {
            nodes.push_back(parseIfExpression(tokens));
        } else {
            cerr << "Parser: unimplemented token type " << tokens[parsingIndex].type << ":" << tokens[parsingIndex].value << endl;
            parsingIndex++;
        }
    }
    cout << "Parsed " << nodes.size() << " nodes" << endl;
    return nodes;
}

vector<ASTNode*> parse(vector<Token> tokens) {
    vector<ASTNode*> nodes = parseWithoutTokenizing(move(tokens));
    initializeModule();
    cout << "Initialized module" << endl;
    for (auto const &node : nodes) {
        cout << node->toString() << endl;
        node->codegen();
    }
    module->print(llvm::errs(), nullptr);
    cout << "Adding return to main" << endl;
    builder->SetInsertPoint(entryBlock);
    builder->CreateRetVoid();
    cout << "Writing object file..." << endl;
    auto targetTriple = llvm::sys::getDefaultTargetTriple();
    llvm::InitializeAllTargetInfos();
    llvm::InitializeAllTargets();
    llvm::InitializeAllTargetMCs();
    llvm::InitializeAllAsmParsers();
    llvm::InitializeAllAsmPrinters();
    string error;
    auto target = llvm::TargetRegistry::lookupTarget(targetTriple, error);
    if (!target) {
        llvm::errs() << error;
        exit(EXIT_FAILURE);
    }
    auto CPU = "generic";
    auto features = "";
    llvm::TargetOptions opt;
    auto RM = llvm::Optional<llvm::Reloc::Model>();
    auto targetMachine = target->createTargetMachine(targetTriple, CPU, features, opt, RM);
    cout << "Setting data layout" << endl;
    module->setDataLayout(targetMachine->createDataLayout());
    module->setTargetTriple(targetTriple);
    auto filename = "output.o";
    error_code EC;
    llvm::raw_fd_ostream dest(filename, EC, llvm::sys::fs::OF_None);
    if (EC) {
        llvm::errs() << "Could not open file: " << EC.message();
        exit(EXIT_FAILURE);
    }
    llvm::legacy::PassManager pass;
    auto fileType = llvm::CGFT_ObjectFile;
    if (targetMachine->addPassesToEmitFile(pass, dest, nullptr, fileType)) {
        llvm::errs() << "Target machine cannot emit a file of this type";
        exit(EXIT_FAILURE);
    }
    cout << "Running pass" << endl;
    pass.run(*module);
    cout << "Pass complete" << endl;
    dest.flush();
    return nodes;
}
