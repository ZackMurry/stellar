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

#include "include/ASTArrayAccess.h"
#include "include/ASTArrayDefinition.h"
#include "include/ASTArrayIndexAssignment.h"
#include "include/ASTBinaryExpression.h"
#include "include/ASTExternDeclaration.h"
#include "include/ASTFunctionDefinition.h"
#include "include/ASTFunctionInvocation.h"
#include "include/ASTIfStatement.h"
#include "include/ASTNumberExpression.h"
#include "include/ASTReturn.h"
#include "include/ASTVariableAssignment.h"
#include "include/ASTVariableDeclaration.h"
#include "include/ASTVariableDefinition.h"
#include "include/ASTVariableExpression.h"

using namespace std;

// todo: boolean literals (true, false)

unsigned long parsingIndex = 0;

void printOutOfTokensError() {
    cerr << "Error: expected token, but nothing found at token " << parsingIndex << endl;
    exit(EXIT_FAILURE);
}

llvm::Type* getLLVMTypeByVariableType(VariableType type, llvm::LLVMContext* context) {
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
        std::cerr << "Parser: unimplemented type " << type << std::endl;
        exit(EXIT_FAILURE);
    }
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
    // Consume "else"
    if (++parsingIndex >= tokens.size()) {
        printOutOfTokensError();
    }
    if (tokens[parsingIndex].type == TOKEN_IF) {
        ASTNode* elseIfExpression = parseIfExpression(tokens);
        vector<ASTNode*> elseBody;
        elseBody.push_back(elseIfExpression);
        return new ASTIfStatement(condition, body, elseBody);
    }
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
    cout << "Initialized module" << endl;
    auto* context = new llvm::LLVMContext();
    auto* module = new llvm::Module("module", *context);
    auto* builder = new llvm::IRBuilder<>(*context);
    llvm::FunctionType* ft = llvm::FunctionType::get(llvm::Type::getVoidTy(*context), false);
    llvm::Function* func = llvm::Function::Create(ft, llvm::Function::ExternalLinkage, "STELLAR_ENTRY", *module);
    llvm::BasicBlock* entryBlock = llvm::BasicBlock::Create(*context, "entry", func);
    builder->SetInsertPoint(entryBlock);
    map<string, llvm::Value*> namedValues;
    for (auto const &node : nodes) {
        cout << node->toString() << endl;
        node->codegen(builder, context, entryBlock, &namedValues, module);
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
