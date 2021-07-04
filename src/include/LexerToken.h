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
    TOKEN_EXTERN
};

struct Token {
    TokenType type;
    std::string value;
};

#endif //STELLAR_LEXERTOKEN_H
