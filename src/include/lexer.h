//
// Created by zack on 7/1/21.
//

#include <vector>
#include "LexerToken.h"

#ifndef STELLAR_LEXER_H
#define STELLAR_LEXER_H

std::vector<Token> tokenize(std::string data);

std::vector<Token> tokenizeFile(const std::string& fileName);

#endif //STELLAR_LEXER_H
