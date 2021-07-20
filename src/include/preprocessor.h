//
// Created by zack on 7/20/21.
//

#include <vector>
#include "LexerToken.h"
#ifndef STELLAR_PREPROCESSOR_H
#define STELLAR_PREPROCESSOR_H

using namespace std;

vector<Token> preprocessTokens(const vector<Token>& tokens, const string& filePath, const string& stdPath);

#endif //STELLAR_PREPROCESSOR_H
