//
// Created by zack on 7/1/21.
//

#include <string>
#ifndef STELLAR_LEXERTOKEN_H
#define STELLAR_LEXERTOKEN_H

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
    TOKEN_NEW
};

struct Token {
    TokenType type;
    std::string value;
    int row;
    int column;
};

#endif //STELLAR_LEXERTOKEN_H
