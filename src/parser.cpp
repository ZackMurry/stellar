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
    VARIABLE_TYPE_B,
    VARIABLE_TYPE_V
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
    llvm::Function* func = llvm::Function::Create(ft, llvm::Function::ExternalLinkage, "main", *module);
    entryBlock = llvm::BasicBlock::Create(*context, "entry", func);
    currBlock = entryBlock;
}

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
    builder->SetInsertPoint(currBlock);
    llvm::AllocaInst* alloca = builder->CreateAlloca(llvmType, nullptr, name);
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

llvm::Type* getLLVMTypeByVariableType(VariableType type) {
    if (type == VARIABLE_TYPE_F) {
        return llvm::Type::getFloatTy(*context);
    } else if (type == VARIABLE_TYPE_D) {
        return llvm::Type::getDoubleTy(*context);
    } else if (type == VARIABLE_TYPE_I32) {
        return llvm::Type::getInt32Ty(*context);
    } else if (type == VARIABLE_TYPE_V) {
        return llvm::Type::getVoidTy(*context);
    } else {
        cerr << "Parser: unimplemented type " << type << endl;
        exit(EXIT_FAILURE);
    }
}

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
    currBlock = entryBlock;
    cout << "Returned" << endl;
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

int getVariableTypeFromToken(const Token& token) {
    if (token.type != TOKEN_IDENTIFIER) {
        return -1;
    }
    // does not include v type
    if (token.value == "i32") {
        return VARIABLE_TYPE_I32;
    } else if (token.value == "f") {
        return VARIABLE_TYPE_F;
    } else if (token.value == "d") {
        return VARIABLE_TYPE_D;
    } else if (token.value == "b") {
        return VARIABLE_TYPE_B;
    } else if (token.value == "v") {
        return VARIABLE_TYPE_V;
    } else {
        return -1;
    }
}

vector<ASTNode*> parseWithoutTokenizing(vector<Token> tokens);

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
    int stackSize = 0;
    unsigned long end = parsingIndex;
    vector<Token> bodyTokens;
    while (stackSize > 0 || (tokens[end].type != TOKEN_PUNCTUATION || tokens[end].value != "}")) {
        if (end + 1 >= tokens.size()) {
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
    cout << "body token size: " << bodyTokens.size() << endl;
    vector<ASTNode*> body = parseWithoutTokenizing(bodyTokens);
    cout << "body size: " << body.size() << endl;
    parsingIndex = end + 1;
    cout << "Arg size before creating: " << args.size() << endl;
    return new ASTFunctionDefinition(name, args, body, type);
}

ASTNode* parseVariableDeclaration(vector<Token> tokens, VariableType type) {
    if (tokens[parsingIndex].type != TOKEN_IDENTIFIER) {
        cerr << "Error: expected identifier after variable type but found token type " << tokens[parsingIndex].type << ":" << tokens[parsingIndex].value << endl;
        exit(EXIT_FAILURE);
    }
    string name = tokens[parsingIndex++].value; // Get var name and consume token
    if (tokens[parsingIndex].type == TOKEN_PUNCTUATION && tokens[parsingIndex].value == "(") {
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

ASTNode* parseIdentifierExpression(vector<Token> tokens) {
    string identifier = tokens[parsingIndex++].value; // Get and consume identifier
    if (identifier == "i32") {
        return parseVariableDeclaration(tokens, VARIABLE_TYPE_I32);
    } else if (identifier == "f") {
        return parseVariableDeclaration(tokens, VARIABLE_TYPE_F);
    } else if (identifier == "v") {
        return parseVariableDeclaration(tokens, VARIABLE_TYPE_V);
    }// todo other types
    if (parsingIndex >= tokens.size()) {
        printOutOfTokensError();
    }
    if (tokens[parsingIndex].type == TOKEN_PUNCTUATION && tokens[parsingIndex].value == "=") {
        return parseVariableAssignment(tokens, identifier);
    }
    if (tokens[parsingIndex].type != TOKEN_PUNCTUATION || tokens[parsingIndex].value != "(") {
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
    // todo implement extern functions
    // currently, function codegen has been rewired to detect if an extern has been declared, but there is no method to declare externs yet
    // plan to use externs for print etc
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
    return nodes;
}
