//
// Created by zack on 7/1/21.
//

#include <vector>
#include <iostream>
#include <fstream>
#include "include/lexer.h"
#include "include/LexerToken.h"

using namespace std;

unsigned long lexingIndex = 0;
string content;

bool isNumeric(string s) {
    // todo negative numbers without messing up '-'
    // todo decimals
    for (char i : s) {
        if (!isdigit(i)) {
            return false;
        }
    }
    return true;
}

struct Token readToken() {
    struct Token token;
    char ch = content.at(lexingIndex);
    if (ch == '=') {
        token.type = TOKEN_PUNCTUATION;
        lexingIndex++;
        if (content.at(lexingIndex) == '=') {
            lexingIndex++;
            token.value = "==";
        } else {
            token.value = "=";
        }
        return token;
    }
    if (ch == '(') {
        token.type = TOKEN_PUNCTUATION;
        token.value = "(";
        lexingIndex++;
        return token;
    }
    if (ch == ')') {
        token.type = TOKEN_PUNCTUATION;
        token.value = ")";
        lexingIndex++;
        return token;
    }
    if (ch == '+') {
        token.type = TOKEN_PUNCTUATION;
        token.value = "+";
        lexingIndex++;
        return token;
    }
    if (ch == '-') {
        token.type = TOKEN_PUNCTUATION;
        token.value = "-";
        lexingIndex++;
        return token;
    }
    if (ch == '*') {
        token.type = TOKEN_PUNCTUATION;
        token.value = "*";
        lexingIndex++;
        return token;
    }
    if (ch == '/') {
        token.type = TOKEN_PUNCTUATION;
        token.value = "/";
        lexingIndex++;
        return token;
    }

    string word;
    while (lexingIndex < content.size() && isalnum(ch)) {
        word += (char) ch;
        if (++lexingIndex >= content.size()) {
            break;
        }
        ch = content.at(lexingIndex);
    }
    if (word == "true") {
        token.type = TOKEN_BOOLEAN;
        token.value = "true";
        return token;
    }
    if (word == "false") {
        token.type = TOKEN_BOOLEAN;
        token.value = "false";
        return token;
    }
    if (isNumeric(word)) {
        token.type = TOKEN_NUMBER;
        token.value = word;
        return token;
    }
    token.type = TOKEN_IDENTIFIER;
    token.value = word;
    return token;
}

vector<Token> tokenize(string data) {
    lexingIndex = 0;
    content = move(data);
    vector<Token> tokens;
    while (lexingIndex < content.size()) {
        struct Token t = readToken();
        cout << t.type << ":" << t.value << endl;
        tokens.push_back(t);
        if (lexingIndex < content.size() && content.at(lexingIndex) == '\n') {
            tokens.push_back({ TOKEN_NEWLINE, "" });
            lexingIndex++;
        }
        while (lexingIndex < content.size() && (content.at(lexingIndex) == ' ' || content.at(lexingIndex) == '\n' || content.at(lexingIndex) == '\t')) {
            lexingIndex++;
        }
    }
    return tokens;
}

vector<Token> tokenizeFile(const string& fileName) {
    ifstream file;
    file.open(fileName);
    if (file.bad()) {
        cerr << "Error while opening file" << endl;
        exit(EXIT_FAILURE);
    }
    string s;
    char ch;
    while ((ch = (char) file.get()) != EOF) {
        s += ch;
    }
    file.close();
    return tokenize(s);
}
