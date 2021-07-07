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

bool isNumeric(const string& s) {
    // todo negative numbers without messing up '-'
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
    if (ch == '{') {
        token.type = TOKEN_PUNCTUATION;
        token.value = "{";
        lexingIndex++;
        return token;
    }
    if (ch == '}') {
        token.type = TOKEN_PUNCTUATION;
        token.value = "}";
        lexingIndex++;
        return token;
    }
    if (ch == '[') {
        token.type = TOKEN_PUNCTUATION;
        token.value = "[";
        lexingIndex++;
        return token;
    }
    if (ch == ']') {
        token.type = TOKEN_PUNCTUATION;
        token.value = "]";
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
    if (ch == ',') {
        token.type = TOKEN_PUNCTUATION;
        token.value = ",";
        lexingIndex++;
        return token;
    }
    if (ch == ';') { // For for-loops
        token.type = TOKEN_PUNCTUATION;
        token.value = ";";
        lexingIndex++;
        return token;
    }
    if (ch == '<') {
        token.type = TOKEN_PUNCTUATION;
        lexingIndex++;
        if (content.at(lexingIndex) == '=') {
            lexingIndex++;
            token.value = "<=";
        } else {
            token.value = "<";
        }
        return token;
    }
    if (ch == '>') {
        token.type = TOKEN_PUNCTUATION;
        lexingIndex++;
        if (content.at(lexingIndex) == '=') {
            lexingIndex++;
            token.value = ">=";
        } else {
            token.value = ">";
        }
        return token;
    }
    if (ch == '!') {
        token.type = TOKEN_PUNCTUATION;
        lexingIndex++;
        if (content.at(lexingIndex) == '=') {
            lexingIndex++;
            token.value = "!=";
        } else {
            token.value = "!";
        }
        return token;
    }
    string word;
    if (isdigit(content.at(lexingIndex))) {
        token.type = TOKEN_NUMBER;
        while (lexingIndex < content.size() && isdigit(ch)) {
            word += (char) ch;
            if (++lexingIndex >= content.size()) {
                break;
            }
            ch = content.at(lexingIndex);
        }
        token.value = word;
        if (content.at(lexingIndex) == '.') {
            token.value += '.';
            while (isdigit(content.at(++lexingIndex))) {
                token.value += content.at(lexingIndex);
            }
        }
        if (content.at(lexingIndex) == 'f' || content.at(lexingIndex) == 'd') {
            token.value += content.at(lexingIndex++);
        } else if (content.at(lexingIndex) == 'i') {
            token.value += content.at(lexingIndex++);
            while (isdigit(content.at(lexingIndex))) {
                token.value += content.at(lexingIndex++);
            }
        }
        return token;
    }

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
    if (word == "r") {
        token.type = TOKEN_RETURN;
        token.value = "";
        return token;
    }
    if (word == "x") {
        token.type = TOKEN_EXTERN;
        token.value = "";
        return token;
    }
    if (word == "if") {
        token.type = TOKEN_IF;
        token.value = "";
        return token;
    }
    if (word == "else") {
        token.type = TOKEN_ELSE;
        token.value = "";
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
        cout << t.type << ":" << t.value << "@" << lexingIndex << endl;
        tokens.push_back(t);
        if (lexingIndex < content.size() && content.at(lexingIndex) == '\n') {
            tokens.push_back({ TOKEN_NEWLINE, "" });
            cout << TOKEN_NEWLINE << ":" << "@" << lexingIndex << endl;
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
