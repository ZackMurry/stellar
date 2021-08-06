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

#include "include/ASTArrayAccess.h"
#include "include/ASTArrayDefinition.h"
#include "include/ASTArrayIndexAssignment.h"
#include "include/ASTBinaryExpression.h"
#include "include/ASTBooleanExpression.h"
#include "include/ASTClassDefinition.h"
#include "include/ASTClassFieldAccess.h"
#include "include/ASTClassFieldStore.h"
#include "include/ASTClassInstantiation.h"
#include "include/ASTExternDeclaration.h"
#include "include/ASTForExpression.h"
#include "include/ASTFunctionDefinition.h"
#include "include/ASTFunctionInvocation.h"
#include "include/ASTIfStatement.h"
#include "include/ASTMethodCall.h"
#include "include/ASTNewExpression.h"
#include "include/ASTNotExpression.h"
#include "include/ASTNullCheckExpression.h"
#include "include/ASTNumberExpression.h"
#include "include/ASTReturn.h"
#include "include/ASTStringExpression.h"
#include "include/ASTVariableAssignment.h"
#include "include/ASTVariableDeclaration.h"
#include "include/ASTVariableDefinition.h"
#include "include/ASTVariableExpression.h"
#include "include/ASTVariableMutation.h"
#include "include/ASTWhileExpression.h"

using namespace std;

// todo: string concatenation (probably using sprintf)
// todo: argv and argc
// todo: exit codes
// todo: explicit casts
// todo: functions with the same name but different signatures
// todo: ternary expressions
// todo: remove requirement for generic types for classes to be defined before the class (currently, Box has to be defined before Container to use Container<Box>)
// todo: inheritance with generics
// todo: @Annotations
// todo: interfaces
// todo: abstract classes

unsigned long parsingIndex = 0;

void printOutOfTokensError() {
    cerr << "Error: expected token, but nothing found at token " << parsingIndex << endl;
    exit(EXIT_FAILURE);
}

void printFatalErrorMessage(const string& s, vector<Token> tokens) {
    cerr << "Error on line " << tokens[parsingIndex].row + 1 << " of " << tokens[parsingIndex].filePath <<  ": " << s << endl;
    exit(EXIT_FAILURE);
}

llvm::Type* getLLVMTypeByPrimitiveVariableType(PrimitiveVariableType type, llvm::LLVMContext* context) {
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
    } else if (type == VARIABLE_TYPE_S) {
        return llvm::Type::getInt8PtrTy(*context);
    } else if (type == VARIABLE_TYPE_B) {
        return llvm::Type::getInt1Ty(*context);
    } else {
        std::cerr << "Parser: unimplemented type " << type << std::endl;
        exit(EXIT_FAILURE);
    }
}

string convertVariableTypeToString(VariableType v) {
    string s = v.type;
    if (!v.genericTypes.empty()) {
        s += "<";
        for (int i = 0; i < v.genericTypes.size(); i++) {
            if (i != 0) {
                s += ",";
            }
            s += convertVariableTypeToString(v.genericTypes.at(i));
        }
        s += ">";
    }
    return s;
}

ASTNode* parseIdentifierExpression(vector<Token> tokens);

ASTNode* parseExpression(const vector<Token>& tokens);

ASTNode* parseNumberExpression(vector<Token> tokens) {
    string val = tokens[parsingIndex++].value;
    PrimitiveVariableType type = VARIABLE_TYPE_I32;
    int lastDigitIndex = -1;
    for (int i = 0; i < val.size(); i++) {
        cout << "i: " << i << endl;
        if (val.at(i) == '.') {
            if (type == VARIABLE_TYPE_F) {
                printFatalErrorMessage("expected a maximum of one decimal to occur in a number", tokens);
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
        printFatalErrorMessage("expected number to start with at least one digit", tokens);
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
            printFatalErrorMessage("unknown explicit variable type suffix", tokens);
        }
    }
    return new ASTNumberExpression(numericPart, type);
}

ASTNode* parseParenExpression(vector<Token> tokens) {
    parsingIndex++; // Consume '('
    auto v = parseExpression(tokens);
    if (tokens[parsingIndex].type != TOKEN_PUNCTUATION || tokens[parsingIndex].value != ")") {
        printFatalErrorMessage("expected ')'", tokens);
    }
    parsingIndex++; // Consume ')'
    return v;
}

// Expects parsing index to be at '<'
vector<VariableType> parseGenericTypes(vector<Token> tokens) {
    vector<VariableType> genericTypes;
    if (tokens[parsingIndex].type == TOKEN_PUNCTUATION && tokens[parsingIndex].value == "<") {
        while (++parsingIndex < tokens.size()) {
            if (tokens[parsingIndex].type != TOKEN_IDENTIFIER) {
                printFatalErrorMessage("expected identifier for class generic type name", tokens);
            }
            cout << "Generic type: " << tokens[parsingIndex].value << endl;
            string identifier = tokens[parsingIndex].value;
            // Consume identifier
            if (++parsingIndex >= tokens.size()) {
                printOutOfTokensError();
            }
            genericTypes.push_back({ identifier, parseGenericTypes(tokens) });
            if (tokens[parsingIndex].type == TOKEN_PUNCTUATION && tokens[parsingIndex].value == ">") {
                if (++parsingIndex >= tokens.size()) {
                    printOutOfTokensError();
                }
                break;
            } else if (tokens[parsingIndex].type != TOKEN_PUNCTUATION && tokens[parsingIndex].value != ",") {
                printFatalErrorMessage("expected ',' or '>' after class generic name", tokens);
            }
        }
    }
    return genericTypes;
}

// Expects parsing index to be at 'new'
ASTNode* parseNewExpression(vector<Token> tokens) {
    cout << "New expression" << endl;
    // Consume 'new'
    if (++parsingIndex >= tokens.size()) {
        printOutOfTokensError();
    }
    if (tokens[parsingIndex].type != TOKEN_IDENTIFIER) {
        printFatalErrorMessage("cannot instantiate unknown class", tokens);
    }
    string className = tokens[parsingIndex].value;

    if (++parsingIndex >= tokens.size()) {
        return new ASTNewExpression({ className, vector<VariableType>() }, vector<ASTNode*>());
    }

    auto genericTypes = parseGenericTypes(tokens);
    if (parsingIndex >= tokens.size()) {
        return new ASTNewExpression({ className, genericTypes }, vector<ASTNode*>());
    }

    if (tokens[parsingIndex].type == TOKEN_PUNCTUATION && tokens[parsingIndex].value == "(") {
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
                    printFatalErrorMessage("expected ',' or ')' in constructor list", tokens);
                }
                // Consume ','
                if (++parsingIndex >= tokens.size()) {
                    printOutOfTokensError();
                }
            }
        }
        // Consume ')'
        parsingIndex++;
        return new ASTNewExpression({ className, genericTypes }, args);
    }
    return new ASTNewExpression({ className, genericTypes }, vector<ASTNode*>());
}

// Parses mutations in the form ++var or --var
ASTNode* parsePreMutationExpression(vector<Token> tokens) {
    cout << "PreMutation" << endl;
    if (tokens[parsingIndex].type != TOKEN_PUNCTUATION) {
        printFatalErrorMessage("internal error. Expected pre-mutation to start with a token of type punctuation", tokens);
    }
    MutationType mutationType;
    if (tokens[parsingIndex].value == "++") {
        mutationType = MUTATION_TYPE_ADD;
    } else if (tokens[parsingIndex].value == "--") {
        mutationType = MUTATION_TYPE_SUB;
    } else {
        printFatalErrorMessage("internal error. Expected pre-mutation to be '++' or '--', but was '" + tokens[parsingIndex].value + "'", tokens);
        return nullptr;
    }
    if (++parsingIndex >= tokens.size()) {
        printOutOfTokensError();
    }
    if (tokens[parsingIndex].type != TOKEN_IDENTIFIER) {
        printFatalErrorMessage("expected an identifier to follow a pre-mutation", tokens);
    }
    string identifier = tokens[parsingIndex++].value;
    return new ASTVariableMutation(identifier, new ASTNumberExpression("1", VARIABLE_TYPE_I32), mutationType, MUTATE_BEFORE);
}

ASTNode* parseBinOpRHS(vector<Token> tokens, int exprPrec, ASTNode* lhs);

ASTNode* parsePrimary(vector<Token> tokens) {
    switch (tokens[parsingIndex].type) {
        case TOKEN_IDENTIFIER:
            return parseIdentifierExpression(tokens);
        case TOKEN_NUMBER:
            return parseNumberExpression(tokens);
        case TOKEN_PUNCTUATION:
            if (tokens[parsingIndex].value == "(") {
                return parseParenExpression(tokens);
            } else if (tokens[parsingIndex].value == "++" || tokens[parsingIndex].value == "--") {
                return parsePreMutationExpression(tokens);
            } else if (tokens[parsingIndex].value == "!") {
                // Consume '!'
                if (++parsingIndex >= tokens.size()) {
                    printOutOfTokensError();
                }
                return new ASTNotExpression(parsePrimary(tokens));
            } else {
                printFatalErrorMessage("unknown token '" + tokens[parsingIndex].value + "' found when parsing expression", tokens);
            }
        case TOKEN_STRING:
            return new ASTStringExpression(tokens[parsingIndex++].value);
        case TOKEN_NEW:
            return parseNewExpression(tokens);
        case TOKEN_BOOLEAN:
            return new ASTBooleanExpression(tokens[parsingIndex++].value == "true");
        default:
            printFatalErrorMessage("unknown token '" + tokens[parsingIndex].value + "' found when parsing expression", tokens);
            return nullptr;
    }
}

int getTokenPrecedence(const Token& token) {
    if (token.type != TOKEN_PUNCTUATION) {
        return -1;
    }
    if (token.value == "||") {
        return 3;
    }
    if (token.value == "&&") {
        return 5;
    }
    if (token.value == "<") {
        return 6;
    }
    if (token.value == ">") {
        return 6;
    }
    if (token.value == "<=") {
        return 6;
    }
    if (token.value == ">=") {
        return 6;
    }
    if (token.value == "==") {
        return 6;
    }
    if (token.value == "!=") {
        return 6;
    }
    if (token.value == "%") {
        return 10;
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
        } else if (tokens[parsingIndex].value == "%") {
            binOp = OPERATOR_MODULO;
        } else if (tokens[parsingIndex].value == "&&") {
            binOp = OPERATOR_AND;
        } else if (tokens[parsingIndex].value == "||") {
            binOp = OPERATOR_OR;
        } else if (tokens[parsingIndex].value == "<") {
            binOp = OPERATOR_LT;
        } else if (tokens[parsingIndex].value == ">") {
            binOp = OPERATOR_GT;
        } else if (tokens[parsingIndex].value == "==") {
            binOp = OPERATOR_EQ;
        } else if (tokens[parsingIndex].value == "<=") {
            binOp = OPERATOR_LE;
        } else if (tokens[parsingIndex].value == ">=") {
            binOp = OPERATOR_GE;
        } else if (tokens[parsingIndex].value == "!=") {
            binOp = OPERATOR_NE;
        } else {
            printFatalErrorMessage("unknown binary operator '" + tokens[parsingIndex].value + "'", tokens);
            return nullptr;
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
    if (parsingIndex >= tokens.size() || tokens[parsingIndex].type == TOKEN_NEWLINE || tokens[parsingIndex].type == TOKEN_EOF || (tokens[parsingIndex].type == TOKEN_PUNCTUATION && tokens[parsingIndex].value == ")")) {
        return lhs;
    }
    if (tokens[parsingIndex].type == TOKEN_PUNCTUATION && tokens[parsingIndex].value == "?") {
        parsingIndex++;
        return new ASTNullCheckExpression(lhs);
    }
    return parseBinOpRHS(tokens, 0, lhs);
}

int getPrimitiveVariableTypeFromString(const string& type) {
    if (type == "i32") {
        return VARIABLE_TYPE_I32;
    } else if (type == "f") {
        return VARIABLE_TYPE_F;
    } else if (type == "d") {
        return VARIABLE_TYPE_D;
    } else if (type == "b") {
        return VARIABLE_TYPE_B;
    } else if (type == "i8") {
        return VARIABLE_TYPE_I8;
    } else if (type == "i16") {
        return VARIABLE_TYPE_I16;
    } else if (type == "i64") {
        return VARIABLE_TYPE_I64;
    } else if (type == "v") {
        return VARIABLE_TYPE_V;
    } else if (type == "s") {
        return VARIABLE_TYPE_S;
    } else {
        return -1;
    }
}
int getPrimitiveVariableTypeFromToken(const Token& token) {
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
    } else if (token.value == "s") {
        return VARIABLE_TYPE_S;
    } else {
        return -1;
    }
}

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
        bodyTokens.push_back(tokens[end++]);
        if (stackSize == 0 && tokens[end].type == TOKEN_PUNCTUATION && tokens[end].value == "}") {
            break;
        }
    }
    cout << "Body token size: " << bodyTokens.size() << endl;
    vector<ASTNode*> body = parse(bodyTokens);
    parsingIndex = end + 1;
    return body;
}

ASTNode* parseClassAccess(vector<Token> tokens, ASTNode* object);

// Expects the parsingIndex to be at an opening parenthesis
ASTNode* parseFunctionDefinition(vector<Token> tokens, const VariableType& returnType, const string& name) {
    cout << "Function definition" << endl;
    vector<ASTVariableDefinition*> args;
    while (++parsingIndex < tokens.size()) {
        if (tokens[parsingIndex].type == TOKEN_PUNCTUATION && tokens[parsingIndex].value == ")") {
            break;
        }
        if (tokens[parsingIndex].type != TOKEN_IDENTIFIER) {
            printFatalErrorMessage("expected type in function parameter", tokens);
        }
        string paramType = tokens[parsingIndex].value;
        cout << "param type " << paramType << endl;
        if (++parsingIndex >= tokens.size()) {
            printOutOfTokensError();
        }
        auto genericTypes = parseGenericTypes(tokens);
        if (tokens[parsingIndex].type != TOKEN_IDENTIFIER) {
            printFatalErrorMessage("expected parameter name after type", tokens);
        }
        args.push_back(new ASTVariableDefinition(tokens[parsingIndex].value, {paramType, genericTypes}));
        cout << "arg: " << args.back()->name << endl;
        if (++parsingIndex >= tokens.size()) {
            printOutOfTokensError();
        }
        if (tokens[parsingIndex].type != TOKEN_PUNCTUATION || tokens[parsingIndex].value != ",") {
            break;
        }
    }
    if (tokens[parsingIndex].type != TOKEN_PUNCTUATION || tokens[parsingIndex].value != ")") {
        printFatalErrorMessage("expected ')' after function parameters", tokens);
    }
    // Consume ')'
    if (++parsingIndex >= tokens.size()) {
        printOutOfTokensError();
    }
    if (tokens[parsingIndex].type != TOKEN_PUNCTUATION || tokens[parsingIndex].value != "{") {
        printFatalErrorMessage("expected '{' after function prototype", tokens);
    }
    // Consume '{'
    if (++parsingIndex >= tokens.size()) {
        printOutOfTokensError();
    }
    vector<ASTNode*> body = getBodyOfBlock(tokens);
    cout << "body size: " << body.size() << endl;
    cout << "Arg size before creating: " << args.size() << endl;
    return new ASTFunctionDefinition(name, args, body, returnType);
}

// Expects parsingIndex to be after ']'
ASTNode* parseArrayAccess(vector<Token> tokens, string name, ASTNode* index) {
    if (tokens[parsingIndex].type == TOKEN_PUNCTUATION && tokens[parsingIndex].value == "=") {
        // Consume '='
        if (++parsingIndex >= tokens.size()) {
            printOutOfTokensError();
        }
        cout << "Parsing arr acc exp" << endl;
        return new ASTArrayIndexAssignment(move(name), index, parseExpression(tokens));
    }
    if (parsingIndex < tokens.size() && tokens[parsingIndex].type == TOKEN_PUNCTUATION && tokens[parsingIndex].value == ".") {
        return parseClassAccess(tokens, new ASTArrayAccess(move(name), index));
    }
    return new ASTArrayAccess(move(name), index);
}

// Expects parsingIndex to be at [
ASTNode* parseArrayDefinition(vector<Token> tokens, const string& type) {
    cout << "Array definition" << endl;
    // Consume '['
    if (++parsingIndex >= tokens.size()) {
        printOutOfTokensError();
    }
    ASTNode* length = parseExpression(tokens);
    cout << "Length: " << length->toString() << endl;
    if (tokens[parsingIndex].type != TOKEN_PUNCTUATION || tokens[parsingIndex].value != "]") {
        printFatalErrorMessage("expected ']' after array length", tokens);
    }
    // Consume ']'
    if (++parsingIndex >= tokens.size() || tokens[parsingIndex].type != TOKEN_IDENTIFIER) {
        return parseArrayAccess(tokens, type, length);
    }
    string name = tokens[parsingIndex++].value; // Consume name
    return new ASTArrayDefinition(name, type, length);
}

ASTNode* parseVariableDeclaration(vector<Token> tokens, const string& type) {
    cout << "Variable declaration: " << parsingIndex << endl;

    auto genericTypes = parseGenericTypes(tokens);

    if (genericTypes.empty() && tokens[parsingIndex].type == TOKEN_PUNCTUATION && tokens[parsingIndex].value == "[") {
        cout << "Array definition" << endl;
        return parseArrayDefinition(tokens, type);
    }
    if (tokens[parsingIndex].type != TOKEN_IDENTIFIER) {
        printFatalErrorMessage("expected identifier after variable type but found token type " + to_string(tokens[parsingIndex].type) + ":" + tokens[parsingIndex].value, tokens);
    }
    string name = tokens[parsingIndex++].value; // Get var name and consume token
    if (genericTypes.empty() && tokens[parsingIndex].type == TOKEN_PUNCTUATION && tokens[parsingIndex].value == "(") {
        cout << "Function definition" << endl;
        // todo returning arrays from functions
        return parseFunctionDefinition(tokens, { type, genericTypes }, name);
    }
    if (tokens[parsingIndex].type != TOKEN_PUNCTUATION || tokens[parsingIndex].value != "=") {
        if (tokens[parsingIndex].type == TOKEN_NEWLINE) {
            parsingIndex++;
            return new ASTVariableDefinition(move(name), { type, genericTypes });
        } else {
            printFatalErrorMessage("expected new line or equals sign after variable definition but instead found token type " + to_string(tokens[parsingIndex].type) + ":" + tokens[parsingIndex].value, tokens);
        }
    }
    parsingIndex++; // Consume '='
    return new ASTVariableDeclaration(name, { type, genericTypes }, parseExpression(tokens));
}

// Expects current token to be the equals sign in 'a = E'
ASTNode* parseVariableAssignment(vector<Token> tokens, string name) {
    if (tokens[parsingIndex].type != TOKEN_PUNCTUATION || tokens[parsingIndex].value != "=") {
        printFatalErrorMessage("expected '='", tokens);
    }
    parsingIndex++; // Consume '='
    return new ASTVariableAssignment(move(name), parseExpression(tokens));
}

// Expects parsingIndex to be at "new"
ASTNode* parseClassInstantiation(vector<Token> tokens) {
    cout << "Class instantiation" << endl;
    // Consume "new"
    if (++parsingIndex >= tokens.size()) {
        printOutOfTokensError();
    }
    if (tokens[parsingIndex].type != TOKEN_IDENTIFIER) {
        printFatalErrorMessage("cannot instantiate unknown class", tokens);
    }
    string className = tokens[parsingIndex].value;
    // Consume class name
    if (++parsingIndex >= tokens.size()) {
        printOutOfTokensError();
    }
    auto genericTypes = parseGenericTypes(tokens);

    string identifier = tokens[parsingIndex].value;
    // Consume identifier
    if (++parsingIndex >= tokens.size()) {
        return new ASTClassInstantiation(className, identifier, vector<ASTNode*>(), genericTypes);
    }
    if (tokens[parsingIndex].type == TOKEN_PUNCTUATION && tokens[parsingIndex].value == "(") {
        // Constructor invocation

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
                    printFatalErrorMessage("expected ',' or ')' in constructor list", tokens);
                }
                // Consume ','
                if (++parsingIndex >= tokens.size()) {
                    printOutOfTokensError();
                }
            }
        }
        parsingIndex++; // Consume ')'
        return new ASTClassInstantiation(className, identifier, args, genericTypes);
    }
    return new ASTClassInstantiation(className, identifier, vector<ASTNode*>(), genericTypes);
}

// Expects parsing index to be at '('
ASTNode* parseMethodCall(vector<Token> tokens, ASTNode* object, const string& methodName) {
    cout << "Method call" << endl;
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
                printFatalErrorMessage("expected ',' or ')' in argument list", tokens);
            }
            // Consume ','
            if (++parsingIndex >= tokens.size()) {
                printOutOfTokensError();
            }
        }
    }
    if (++parsingIndex >= tokens.size()) {
        return new ASTMethodCall(object, methodName, args);
    }
    if (tokens[parsingIndex].type == TOKEN_PUNCTUATION && tokens[parsingIndex].value == ".") {
        return parseClassAccess(tokens, new ASTMethodCall(object, methodName, args));
    }
    return new ASTMethodCall(object, methodName, args);
}

// Expects parsingIndex to be at '.'
ASTNode* parseClassAccess(vector<Token> tokens, ASTNode* object) {
    cout << "Class access: " << endl;
    // Consume '.'
    if (++parsingIndex >= tokens.size()) {
        printOutOfTokensError();
    }
    if (tokens[parsingIndex].type != TOKEN_IDENTIFIER && tokens[parsingIndex].type != TOKEN_CLASS) {
        printFatalErrorMessage("expected field/method name after '.' in object access", tokens);
    }
    string fieldName = tokens[parsingIndex].value;
    if (tokens[parsingIndex].type == TOKEN_CLASS) {
        fieldName = "class";
    }
    if (++parsingIndex >= tokens.size()) {
        return new ASTClassFieldAccess(object, fieldName);
    }
    if (tokens[parsingIndex].type != TOKEN_PUNCTUATION || tokens[parsingIndex].value != "=") {
        if (tokens[parsingIndex].type == TOKEN_PUNCTUATION && tokens[parsingIndex].value == "(") {
            return parseMethodCall(tokens, object, fieldName);
        } else if (tokens[parsingIndex].type == TOKEN_PUNCTUATION && tokens[parsingIndex].value == ".") {
            return parseClassAccess(tokens, new ASTClassFieldAccess(object, fieldName));
        }
        return new ASTClassFieldAccess(object, fieldName);
    }
    cout << "Class field store" << endl;
    // Consume '='
    if (++parsingIndex >= tokens.size()) {
        printOutOfTokensError();
    }
    return new ASTClassFieldStore(object, fieldName, parseExpression(tokens));
}

// Expects parsing index to be after identifier
ASTNode* parseVariableMutation(vector<Token> tokens, const string& identifier) {
    if (tokens[parsingIndex].type != TOKEN_PUNCTUATION) {
        printFatalErrorMessage("internal error. Expected variable mutation token to be of type punctuation", tokens);
    }
    ASTNode* change;
    MutationType mutationType;
    if (tokens[parsingIndex].value == "++") {
        change = new ASTNumberExpression("1", VARIABLE_TYPE_I32);
        mutationType = MUTATION_TYPE_ADD;
    } else if (tokens[parsingIndex].value == "--") {
        change = new ASTNumberExpression("1", VARIABLE_TYPE_I32);
        mutationType = MUTATION_TYPE_SUB;
    } else if (tokens[parsingIndex].value == "+=") {
        if (++parsingIndex >= tokens.size()) {
            printOutOfTokensError();
        }
        change = parseExpression(tokens);
        mutationType = MUTATION_TYPE_ADD;
    } else if (tokens[parsingIndex].value == "-=") {
        if (++parsingIndex >= tokens.size()) {
            printOutOfTokensError();
        }
        change = parseExpression(tokens);
        mutationType = MUTATION_TYPE_SUB;
    } else if (tokens[parsingIndex].value == "*=") {
        if (++parsingIndex >= tokens.size()) {
            printOutOfTokensError();
        }
        change = parseExpression(tokens);
        mutationType = MUTATION_TYPE_MUL;
    } else if (tokens[parsingIndex].value == "/=") {
        if (++parsingIndex >= tokens.size()) {
            printOutOfTokensError();
        }
        change = parseExpression(tokens);
        mutationType = MUTATION_TYPE_DIV;
    } else {
        printFatalErrorMessage("internal error. Expected variable mutation to be valid, but instead found '" + tokens[parsingIndex].value + "'", tokens);
        return nullptr;
    }
    parsingIndex++;
    return new ASTVariableMutation(identifier, change, mutationType, MUTATE_AFTER);
}

bool isGenericExpression(vector<Token> tokens) {
    if (tokens[parsingIndex].type == TOKEN_PUNCTUATION && tokens[parsingIndex].value == "<") {
        while (++parsingIndex < tokens.size()) {
            if (tokens[parsingIndex].type != TOKEN_IDENTIFIER) {
                return false;
            }
            string identifier = tokens[parsingIndex].value;
            // Consume identifier
            if (++parsingIndex >= tokens.size()) {
                return false;
            }
            if (tokens[parsingIndex].type == TOKEN_PUNCTUATION && tokens[parsingIndex].value == "<" && !isGenericExpression(tokens)) {
                return false;
            }
            if (tokens[parsingIndex].type == TOKEN_PUNCTUATION && tokens[parsingIndex].value == ">") {
                if (++parsingIndex >= tokens.size()) {
                    return false;
                }
                break;
            } else if (tokens[parsingIndex].type != TOKEN_PUNCTUATION && tokens[parsingIndex].value != ",") {
                return false;
            }
        }
    }
    return true;
}

ASTNode* parseIdentifierExpression(vector<Token> tokens) {
    string identifier = tokens[parsingIndex].value; // Get and consume identifier
    if (++parsingIndex >= tokens.size()) {
        return new ASTVariableExpression(identifier);
    }
    bool isGeneric = false;
    if (tokens[parsingIndex].type == TOKEN_PUNCTUATION && tokens[parsingIndex].value == "<") {
        unsigned long pi = parsingIndex;
        isGeneric = isGenericExpression(tokens);
        parsingIndex = pi;
    }
    cout << "isGeneric: " << isGeneric << endl;
    if (tokens[parsingIndex].type == TOKEN_IDENTIFIER || (tokens[parsingIndex].type == TOKEN_PUNCTUATION && tokens[parsingIndex].value == "[") || isGeneric) {
        return parseVariableDeclaration(tokens, identifier);
    }
    if (tokens[parsingIndex].type == TOKEN_PUNCTUATION && tokens[parsingIndex].value == "=") {
        return parseVariableAssignment(tokens, identifier);
    } else if (tokens[parsingIndex].type == TOKEN_PUNCTUATION && tokens[parsingIndex].value == ".") {
        return parseClassAccess(tokens, new ASTVariableExpression(identifier));
    } else if (tokens[parsingIndex].type == TOKEN_PUNCTUATION) {
        if (tokens[parsingIndex].value == "++" ||
            tokens[parsingIndex].value == "--" ||
            tokens[parsingIndex].value == "+=" ||
            tokens[parsingIndex].value == "-=" ||
            tokens[parsingIndex].value == "*=" ||
            tokens[parsingIndex].value == "/=") {
            cout << "Variable mutation" << endl;
            return parseVariableMutation(tokens, identifier);
        }
    }
    if (tokens[parsingIndex].type != TOKEN_PUNCTUATION || tokens[parsingIndex].value != "(") {
        cout << "Variable expression" << endl;
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
                printFatalErrorMessage("expected ',' or ')' in argument list", tokens);
            }
            // Consume ','
            if (++parsingIndex >= tokens.size()) {
                printOutOfTokensError();
            }
        }
    }
    if (++parsingIndex >= tokens.size()) {
        return new ASTFunctionInvocation(identifier, args);
    }
    if (tokens[parsingIndex].type == TOKEN_PUNCTUATION && tokens[parsingIndex].value == ".") {
        return parseClassAccess(tokens, new ASTFunctionInvocation(identifier, args));
    }
    return new ASTFunctionInvocation(identifier, args);
}

ASTNode* parseReturnExpression(vector<Token> tokens) {
    cout << "ret" << endl;
    // Consume 'ret'
    if (++parsingIndex >= tokens.size()) {
        printOutOfTokensError();
    }
    if (tokens[parsingIndex].type == TOKEN_NEWLINE) {
        return new ASTReturn(nullptr);
    }
    return new ASTReturn(parseExpression(tokens));
}

// Todo make parameter names optional
ASTNode* parseExternExpression(vector<Token> tokens) {
    cout << "Extern definition" << endl;
    // Consume 'x'
    if (++parsingIndex >= tokens.size()) {
        printOutOfTokensError();
    }
    if (tokens[parsingIndex].type != TOKEN_IDENTIFIER) {
        printFatalErrorMessage("expected function return type after 'x'", tokens);
    }
    int returnType = getPrimitiveVariableTypeFromToken(tokens[parsingIndex]);
    if (returnType == -1) {
        printFatalErrorMessage("expected function return type after 'x' but instead found " + tokens[parsingIndex].value, tokens);
    }
    if (++parsingIndex >= tokens.size()) {
        printOutOfTokensError();
    }
    if (tokens[parsingIndex].type != TOKEN_IDENTIFIER) {
        printFatalErrorMessage("expected extern function name after its return type", tokens);
    }
    string name = tokens[parsingIndex].value;
    // Consume name
    if (++parsingIndex >= tokens.size()) {
        printOutOfTokensError();
    }
    vector<string> argTypes;
    bool isVarArgs = false;
    while (++parsingIndex < tokens.size()) {
        if (tokens[parsingIndex].type == TOKEN_PUNCTUATION && tokens[parsingIndex].value == ")") {
            break;
        }
        if (tokens[parsingIndex].type == TOKEN_PUNCTUATION && tokens[parsingIndex].value == ".") {
            // Var args
            if (++parsingIndex >= tokens.size()) {
                printOutOfTokensError();
            }
            if (tokens[parsingIndex].type != TOKEN_PUNCTUATION || tokens[parsingIndex].value != ".") {
                printFatalErrorMessage("unexpected '.' in extern definition. Did you mean '...'?", tokens);
            }
            if (++parsingIndex >= tokens.size()) {
                printOutOfTokensError();
            }
            if (tokens[parsingIndex].type != TOKEN_PUNCTUATION || tokens[parsingIndex].value != ".") {
                printFatalErrorMessage("unexpected '..' in extern definition. Did you mean '...'?", tokens);
            }
            if (++parsingIndex >= tokens.size()) {
                printOutOfTokensError();
            }
            cout << "Var args" << endl;
            isVarArgs = true;
            break;
        }
        if (tokens[parsingIndex].type != TOKEN_IDENTIFIER) {
            printFatalErrorMessage("expected type in function parameter", tokens);
        }
        string type = tokens[parsingIndex].value;
        if (++parsingIndex >= tokens.size()) {
            printOutOfTokensError();
        }
        // Consume param name if present
        if (tokens[parsingIndex].type == TOKEN_IDENTIFIER) {
            if (++parsingIndex >= tokens.size()) {
                printOutOfTokensError();
            }
        }
        argTypes.push_back(type);
        if (tokens[parsingIndex].type != TOKEN_PUNCTUATION || tokens[parsingIndex].value != ",") {
            break;
        }
    }
    if (tokens[parsingIndex].type != TOKEN_PUNCTUATION || tokens[parsingIndex].value != ")") {
        printFatalErrorMessage("expected ')' after function parameters", tokens);
    }
    // Consume ')'
    parsingIndex++;
    return new ASTExternDeclaration(name, argTypes, (PrimitiveVariableType) returnType, isVarArgs);
}

// Expects parsing index to be at "if"
ASTNode* parseIfExpression(vector<Token> tokens) {
    cout << "If expression" << endl;
    // Consume "if"
    if (++parsingIndex >= tokens.size()) {
        printOutOfTokensError();
    }
    if (tokens[parsingIndex].type != TOKEN_PUNCTUATION || tokens[parsingIndex].value != "(") {
        printFatalErrorMessage("expected '(' after if", tokens);
    }
    // Consume "("
    if (++parsingIndex >= tokens.size()) {
        printOutOfTokensError();
    }
    auto* condition = parseExpression(tokens);
    if (condition == nullptr) {
        exit(EXIT_FAILURE);
    }
    cout << "Condition: " << condition->toString() << endl;

    if (tokens[parsingIndex].type != TOKEN_PUNCTUATION || tokens[parsingIndex].value != ")") {
        printFatalErrorMessage("expected ')' after if condition", tokens);
    }
    // Consume ")"
    if (++parsingIndex >= tokens.size()) {
        printOutOfTokensError();
    }

    if (tokens[parsingIndex].type != TOKEN_PUNCTUATION || tokens[parsingIndex].value != "{") {
        printFatalErrorMessage("expected '{' after if condition (single-line if statements without braces are not allowed)", tokens);
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
        printFatalErrorMessage("expected '{' after else (single-line else blocks without braces are not allowed)", tokens);
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

// expects parsingIndex to be at "class"
ASTNode* parseClassDefinition(vector<Token> tokens) {
    cout << "Class definition" << endl;
    // Consume "class"
    if (++parsingIndex >= tokens.size()) {
        printOutOfTokensError();
    }
    if (tokens[parsingIndex].type != TOKEN_IDENTIFIER) {
        printFatalErrorMessage("expected class name after \"class\"", tokens);
    }
    string name = tokens[parsingIndex].value;
    // Consume name
    if (++parsingIndex >= tokens.size()) {
        printOutOfTokensError();
    }
    // Generics
    auto genericTypes = parseGenericTypes(tokens);

    string parentClass;
    if (tokens[parsingIndex].type == TOKEN_EXTENDS) {
        // Consume "extends"
        if (++parsingIndex >= tokens.size()) {
            printOutOfTokensError();
        }
        if (tokens[parsingIndex].type != TOKEN_IDENTIFIER) {
            printFatalErrorMessage("expected class name after \"extends\"", tokens);
        }
        parentClass = tokens[parsingIndex].value;

        // Consume parent class name
        if (++parsingIndex >= tokens.size()) {
            printOutOfTokensError();
        }
    }

    if (tokens[parsingIndex].type != TOKEN_PUNCTUATION || tokens[parsingIndex].value != "{") {
        printFatalErrorMessage("expected '{' after class name", tokens);
    }
    // Consume '{'
    if (++parsingIndex >= tokens.size()) {
        printOutOfTokensError();
    }
    if (tokens[parsingIndex].type != TOKEN_NEWLINE) {
        printFatalErrorMessage("expected newline after class opening brace", tokens);
    }
    // Consume newline
    if (++parsingIndex >= tokens.size()) {
        printOutOfTokensError();
    }
    vector<ClassFieldDefinition> fields;
    vector<ASTFunctionDefinition*> methods;
    while (parsingIndex < tokens.size() && (tokens[parsingIndex].type != TOKEN_PUNCTUATION || tokens[parsingIndex].value != "}")) {
        if (tokens[parsingIndex].type != TOKEN_IDENTIFIER && tokens[parsingIndex].type != TOKEN_NEW) {
            printFatalErrorMessage("expected identifier in class body", tokens);
        }
        string fieldType = tokens[parsingIndex].value;
        bool isConstructor = false;
        if (tokens[parsingIndex].type == TOKEN_NEW) {
            fieldType = "v";
            isConstructor = true;
        }
        if (++parsingIndex >= tokens.size()) {
            printOutOfTokensError();
        }
        auto fieldGenericTypes = parseGenericTypes(tokens);
        string fieldName;
        if (isConstructor) {
            fieldName = "new";
        } else {
            if (tokens[parsingIndex].type != TOKEN_IDENTIFIER) {
                if (fieldType == "v" && tokens[parsingIndex].type == TOKEN_NEW) {
                    // Allow "v new(...) {...}"
                    isConstructor = true;
                    fieldName = "new";
                } else {
                    printFatalErrorMessage("expected identifier for field name", tokens);
                }
            } else {
                fieldName = tokens[parsingIndex].value;
            }
            // Consume field name
            if (++parsingIndex >= tokens.size()) {
                printOutOfTokensError();
            }
        }
        if (tokens[parsingIndex].type == TOKEN_PUNCTUATION && tokens[parsingIndex].value == "(") {
            cout << "method: " << fieldName << endl;
            methods.push_back((ASTFunctionDefinition *) parseFunctionDefinition(tokens, {fieldType, fieldGenericTypes}, fieldName));
        } else if (isConstructor) {
            printFatalErrorMessage("A field cannot have the type 'new'", tokens);
        } else {
            cout << "field: " << fieldName << endl;
            fields.push_back({fieldName, fieldType, fieldGenericTypes});
        }
        if (tokens[parsingIndex].type != TOKEN_NEWLINE) {
            printFatalErrorMessage("expected new line after field or method declaration", tokens);
        }
        // Consume newline
        if (++parsingIndex >= tokens.size()) {
            printOutOfTokensError();
        }
    }
    if (tokens[parsingIndex].type == TOKEN_PUNCTUATION && tokens[parsingIndex].value == "}") {
        // Consume }
        if (++parsingIndex >= tokens.size()) {
            printOutOfTokensError();
        }
        cout << "class" << endl;
        return new ASTClassDefinition(name, fields, methods, genericTypes, parentClass);
    } else {
        printFatalErrorMessage("expected empty class", tokens);
        return nullptr;
    }
}

// todo: break, continue
ASTNode* parseForExpression(vector<Token> tokens) {
    cout << "For expression" << endl;
    // Consume "for"
    if (++parsingIndex >= tokens.size()) {
        printOutOfTokensError();
    }
    if (tokens[parsingIndex].type != TOKEN_PUNCTUATION || tokens[parsingIndex].value != "(") {
        printFatalErrorMessage("expected '(' after \"for\"", tokens);
    }
    // Consume '('
    if (++parsingIndex >= tokens.size()) {
        printOutOfTokensError();
    }
    vector<Token> initTokens; // Tokens associated with the initializer
    while (parsingIndex < tokens.size() && (tokens[parsingIndex].type != TOKEN_PUNCTUATION || tokens[parsingIndex].value != ";")) {
        initTokens.push_back(tokens[parsingIndex]);
        parsingIndex++;
    }
    // Check for overflow and consume ';'
    if (++parsingIndex >= tokens.size()) {
        printOutOfTokensError();
    }
    unsigned long currParsingIndex = parsingIndex;
    vector<ASTNode*> initNodes = parse(initTokens);
    parsingIndex = currParsingIndex;
    if (initNodes.size() > 1) {
        printFatalErrorMessage("expected one statement in for loop initializer", tokens);
    }
    ASTNode* init = nullptr;
    if (!initNodes.empty()) {
        init = initNodes.at(0);
    }
    // If there is a condition
    ASTNode* condition = nullptr;
    if (tokens[parsingIndex].type != TOKEN_PUNCTUATION || tokens[parsingIndex].value != ";") {
        condition = parseExpression(tokens);
        if (condition == nullptr) {
            exit(EXIT_FAILURE);
        }
        cout << "Condition: " << condition->toString() << endl;
        if (tokens[parsingIndex].type != TOKEN_PUNCTUATION || tokens[parsingIndex].value != ";") {
            printFatalErrorMessage("expected ';' after condition in for loop", tokens);
        }
    }
    if (++parsingIndex >= tokens.size()) {
        printOutOfTokensError();
    }
    vector<Token> actionTokens; // Tokens associated with the initializer
    int numParensOnStack = 0;
    while (parsingIndex < tokens.size() && (tokens[parsingIndex].type != TOKEN_PUNCTUATION || tokens[parsingIndex].value != ")" || numParensOnStack != 0)) {
        if (tokens[parsingIndex].type == TOKEN_PUNCTUATION && tokens[parsingIndex].value == "(") {
            cout << "'('" << endl;
            numParensOnStack++;
        } else if (tokens[parsingIndex].type == TOKEN_PUNCTUATION && tokens[parsingIndex].value == ")") {
            cout << "')'" << endl;
            numParensOnStack--;
        }
        actionTokens.push_back(tokens[parsingIndex]);
        parsingIndex++;
    }
    // Check for overflow and consume ')'
    if (++parsingIndex >= tokens.size()) {
        printOutOfTokensError();
    }
    currParsingIndex = parsingIndex;
    vector<ASTNode*> actionNodes = parse(actionTokens);
    parsingIndex = currParsingIndex;
    if (actionNodes.size() > 1) {
        printFatalErrorMessage("expected one statement in for loop action", tokens);
    }
    ASTNode* action = nullptr;
    if (!actionNodes.empty()) {
        action = actionNodes.at(0);
        cout << "Action: " << action->toString() << endl;
    }
    if (tokens[parsingIndex].type != TOKEN_PUNCTUATION || tokens[parsingIndex].value != "{") {
        printFatalErrorMessage("expected '{' after for loop start", tokens);
    }
    // Consume '{'
    if (++parsingIndex >= tokens.size()) {
        printOutOfTokensError();
    }
    vector<ASTNode*> body = getBodyOfBlock(tokens);
    return new ASTForExpression(init, condition, action, body);
}

ASTNode* parseWhileExpression(vector<Token> tokens) {
    cout << "While expression" << endl;
    // Consume "while"
    if (++parsingIndex >= tokens.size()) {
        printOutOfTokensError();
    }
    if (tokens[parsingIndex].type != TOKEN_PUNCTUATION || tokens[parsingIndex].value != "(") {
        printFatalErrorMessage("expected '(' after while", tokens);
    }
    // Consume "("
    if (++parsingIndex >= tokens.size()) {
        printOutOfTokensError();
    }
    auto* condition = parseExpression(tokens);
    if (condition == nullptr) {
        exit(EXIT_FAILURE);
    }
    cout << "Condition: " << condition->toString() << endl;

    if (tokens[parsingIndex].type != TOKEN_PUNCTUATION || tokens[parsingIndex].value != ")") {
        printFatalErrorMessage("expected ')' after while condition", tokens);
    }
    // Consume ")"
    if (++parsingIndex >= tokens.size()) {
        printOutOfTokensError();
    }

    if (tokens[parsingIndex].type != TOKEN_PUNCTUATION || tokens[parsingIndex].value != "{") {
        printFatalErrorMessage("expected '{' after while condition (single-line while statements without braces are not allowed)", tokens);
    }
    // Consume "{"
    if (++parsingIndex >= tokens.size()) {
        printOutOfTokensError();
    }
    vector<ASTNode*> body = getBodyOfBlock(tokens);
    return new ASTWhileExpression(condition, body);
}

vector<ASTNode*> parse(vector<Token> tokens) {
    parsingIndex = 0;
    vector<ASTNode*> nodes;
    while (parsingIndex < tokens.size()) {
        cout << "Next node" << endl;
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
        } else if (tokens[parsingIndex].type == TOKEN_CLASS) {
            nodes.push_back(parseClassDefinition(tokens));
        } else if (tokens[parsingIndex].type == TOKEN_NEW) {
            nodes.push_back(parseClassInstantiation(tokens));
        } else if (tokens[parsingIndex].type == TOKEN_FOR) {
            nodes.push_back(parseForExpression(tokens));
        } else if (tokens[parsingIndex].type == TOKEN_WHILE) {
            nodes.push_back(parseWhileExpression(tokens));
        } else if (tokens[parsingIndex].type == TOKEN_PUNCTUATION && (tokens[parsingIndex].value == "++" || tokens[parsingIndex].value == "--")) {
            nodes.push_back(parsePreMutationExpression(tokens));
        } else {
            printFatalErrorMessage("unknown token type " + to_string(tokens[parsingIndex].type) + ":" + tokens[parsingIndex].value, tokens);
        }
    }
    cout << "Parsed " << nodes.size() << " nodes" << endl;
    return nodes;
}
