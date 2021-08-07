//
// Created by zack on 7/1/21.
//

#include <string>
#ifndef STELLAR_LEXERTOKEN_H
#define STELLAR_LEXERTOKEN_H

using namespace std;

enum TokenType {
    TOKEN_EOF,
    TOKEN_IDENTIFIER,
    TOKEN_NUMBER,
    TOKEN_PUNCTUATION,
    TOKEN_BOOLEAN,
    TOKEN_NEWLINE,
    TOKEN_RETURN,
    TOKEN_EXTERN,
    TOKEN_IF,
    TOKEN_ELSE,
    TOKEN_STRING,
    TOKEN_CLASS,
    TOKEN_NEW,
    TOKEN_IMPORT,
    TOKEN_FOR,
    TOKEN_WHILE,
    TOKEN_EXTENDS,
    TOKEN_VIRTUAL,
    TOKEN_OVERRIDE
};

struct Token {
    TokenType type;
    string value;
    int row;
    int column;
    string filePath;
};

#endif //STELLAR_LEXERTOKEN_H
