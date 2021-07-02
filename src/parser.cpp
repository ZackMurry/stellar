//
// Created by zack on 7/1/21.
//

#include <utility>
#include <vector>
#include <iostream>
#include <map>
#include "include/parser.h"
#include "llvm/IR/IRBuilder.h"

using namespace std;

unsigned long parsingIndex = 0;

static llvm::LLVMContext* context;
static llvm::IRBuilder<>* builder;
static llvm::Module* module;
static map<string, llvm::Value*> namedValues;

void printOutOfTokensError() {
    cerr << "Error: expected token, but nothing found";
    exit(EXIT_FAILURE);
}

enum ExpressionOperator {
    OPERATOR_PLUS,
    OPERATOR_MINUS,
    OPERATOR_TIMES,
    OPERATOR_DIVIDE
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
    }
}

enum VariableType {
    VARIABLE_TYPE_I8,
    VARIABLE_TYPE_I16,
    VARIABLE_TYPE_I32,
    VARIABLE_TYPE_I64,
    VARIABLE_TYPE_F,
    VARIABLE_TYPE_D,
    VARIABLE_TYPE_B
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
};

llvm::Value* ASTVariableDefinition::codegen() {
    llvm::Type* llvmType;
    if (type == VARIABLE_TYPE_F) {
        llvmType = llvm::Type::getFloatTy(*context);
    } else if (type == VARIABLE_TYPE_D) {
        llvmType = llvm::Type::getDoubleTy(*context);
    } else if (type == VARIABLE_TYPE_I32) {
        llvmType = llvm::Type::getInt32Ty(*context);
    } else {
        cerr << "Parser: unimplemented type " << type << endl;
        exit(EXIT_FAILURE);
    }
    llvm::AllocaInst* inst = builder->CreateAlloca(llvmType, nullptr, name);
    builder->CreateLoad(inst);
    return inst;
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

static llvm::BasicBlock* entryBlock;

void createEntryFunction() {
    llvm::FunctionType* ft = llvm::FunctionType::get(llvm::Type::getVoidTy(*context), false);
    llvm::Function* func = llvm::Function::Create(ft, llvm::Function::ExternalLinkage, "main", *module);
    entryBlock = llvm::BasicBlock::Create(*context, "entry", func);
}


llvm::Value* ASTVariableDeclaration::codegen() {
    llvm::Type* llvmType;
    if (type == VARIABLE_TYPE_F) {
        llvmType = llvm::Type::getFloatTy(*context);
    } else if (type == VARIABLE_TYPE_D) {
        llvmType = llvm::Type::getDoubleTy(*context);
    } else if (type == VARIABLE_TYPE_I32) {
        llvmType = llvm::Type::getInt32Ty(*context);
    } else {
        cerr << "Parser: unimplemented type " << type << endl;
        exit(EXIT_FAILURE);
    }
    cout << "CreateAlloca" << endl;
    cout << "Context: " << context << endl;
    builder->SetInsertPoint(entryBlock);
    llvm::AllocaInst* alloca = builder->CreateAlloca(llvmType, 0, name.c_str());
    cout << "CreateStore" << endl;
    builder->CreateStore(value->codegen(), alloca);
    return alloca;
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
    double val;
public:
    explicit ASTNumberExpression(double val) : val(val) {}
    string toString() override {
        return "[NUM_EXP: " + to_string(val) + "]";
    }
    llvm::Value *codegen() override;
};

llvm::Value* ASTNumberExpression::codegen() {
    // todo i vs f inference on number literals
    return llvm::ConstantFP::get(*context, llvm::APFloat(val));
}

ASTNode* parseIdentifierExpression(vector<Token> tokens);

ASTNode* parseExpression(const vector<Token>& tokens);

ASTNode* parseNumberExpression(vector<Token> tokens) {
    double val = strtod(tokens[parsingIndex++].value.c_str(), nullptr);
    return new ASTNumberExpression(val);
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
                cerr << "Error: unknown token found when parsing expression" << endl;
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
    // todo other tokens
    return -1;
}

ASTNode* parseBinOpRHS(vector<Token> tokens, int exprPrec, ASTNode* lhs) {
    cout << "Parsing binary operator RHS" << endl;
    while (true) {
        if (tokens[parsingIndex].type == TOKEN_EOF || tokens[parsingIndex].type == TOKEN_NEWLINE) {
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
            cerr << "Error: unknown binary operator (" << tokens[parsingIndex].value << endl;
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

ASTNode* parseVariableDeclaration(vector<Token> tokens, VariableType type) {
    if (tokens[parsingIndex].type != TOKEN_IDENTIFIER) {
        cerr << "Error: expected identifier after variable type but found token type " << tokens[parsingIndex].type << ":" << tokens[parsingIndex].value << endl;
        exit(EXIT_FAILURE);
    }
    string name = tokens[parsingIndex++].value; // Get var name and consume token
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

ASTNode* parseIdentifierExpression(vector<Token> tokens) {
    string identifier = tokens[parsingIndex++].value; // Get and consume identifier
    if (identifier == "i32") {
        return parseVariableDeclaration(tokens, VARIABLE_TYPE_I32);
    } else if (identifier == "f") {
        return parseVariableDeclaration(tokens, VARIABLE_TYPE_F);
    }// todo other types
    if (parsingIndex >= tokens.size()) {
        printOutOfTokensError();
    }
    if (tokens[parsingIndex].type != TOKEN_PUNCTUATION || tokens[parsingIndex].value != "(") {
        return new ASTVariableExpression(identifier);
    }
    // todo function invocations
    cerr << "Error: unimplemented function invocation" << endl;
    exit(EXIT_FAILURE);
}

void initializeModule() {
    context = new llvm::LLVMContext();
    module = new llvm::Module("module", *context);
    builder = new llvm::IRBuilder<>(*context);
    createEntryFunction();
}

vector<ASTNode*> parse(vector<Token> tokens) {
    parsingIndex = 0;
    vector<ASTNode*> nodes;
    while (parsingIndex < tokens.size()) {
        if (tokens[parsingIndex].type == TOKEN_IDENTIFIER) {
            nodes.push_back(parseIdentifierExpression(tokens));
        } else if (tokens[parsingIndex].type == TOKEN_EOF || tokens[parsingIndex].type == TOKEN_NEWLINE) {
            parsingIndex++;
        } else {
            cerr << "Parser: unimplemented token type " << tokens[parsingIndex].type << ":" << tokens[parsingIndex].value << endl;
            parsingIndex++;
        }
    }
    cout << "Parsed " << nodes.size() << " nodes" << endl;
    initializeModule();
    cout << "Initialized module" << endl;
    for (auto const &node : nodes) {
        cout << node->toString() << endl;
        node->codegen();
    }
    module->print(llvm::errs(), nullptr);
    return nodes;
}
