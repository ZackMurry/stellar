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
int row = 0;
int column = 0;
string content;
string filePath;

bool isNumeric(const string& s) {
    // todo negative numbers without messing up '-'
    for (char i : s) {
        if (!isdigit(i)) {
            return false;
        }
    }
    return true;
}

void consumeChar() {
    lexingIndex++;
    column++;
    if (lexingIndex > content.size()) {
        cout << "Warn: consume char exceeded content size" << endl;
    }
}

struct Token readToken() {
    struct Token token;
    token.row = row;
    token.column = column;
    token.filePath = filePath;
    char ch = content.at(lexingIndex);

    if (ch == '/') {
        if (lexingIndex + 1 >= content.size()) {
            cerr << "Error: unexpected end of file" << endl;
            exit(EXIT_FAILURE);
        }
        if (content.at(lexingIndex + 1) == '/') {
            consumeChar(); // Consume first '/'
            consumeChar(); // Consume second '/'
            while (lexingIndex < content.size() && content.at(lexingIndex) != '\n' && content.at(lexingIndex) != EOF) {
                consumeChar();
            }
            consumeChar(); // Consume newline
            token.type = TOKEN_NEWLINE;
            row++;
            column = 0;
            return token;
        }
    }

    if (ch == '=') {
        token.type = TOKEN_PUNCTUATION;
        consumeChar();
        if (content.at(lexingIndex) == '=') {
            consumeChar();
            token.value = "==";
        } else {
            token.value = "=";
        }
        return token;
    }
    if (ch == '(') {
        token.type = TOKEN_PUNCTUATION;
        token.value = "(";
        consumeChar();
        return token;
    }
    if (ch == ')') {
        token.type = TOKEN_PUNCTUATION;
        token.value = ")";
        consumeChar();
        return token;
    }
    if (ch == '{') {
        token.type = TOKEN_PUNCTUATION;
        token.value = "{";
        consumeChar();
        return token;
    }
    if (ch == '}') {
        token.type = TOKEN_PUNCTUATION;
        token.value = "}";
        consumeChar();
        return token;
    }
    if (ch == '[') {
        token.type = TOKEN_PUNCTUATION;
        token.value = "[";
        consumeChar();
        return token;
    }
    if (ch == ']') {
        token.type = TOKEN_PUNCTUATION;
        token.value = "]";
        consumeChar();
        return token;
    }
    if (ch == '+') {
        token.type = TOKEN_PUNCTUATION;
        token.value = "+";
        consumeChar();
        return token;
    }
    if (ch == '-') {
        token.type = TOKEN_PUNCTUATION;
        token.value = "-";
        consumeChar();
        return token;
    }
    if (ch == '*') {
        token.type = TOKEN_PUNCTUATION;
        token.value = "*";
        consumeChar();
        return token;
    }
    if (ch == '/') {
        token.type = TOKEN_PUNCTUATION;
        token.value = "/";
        consumeChar();
        return token;
    }
    if (ch == ',') {
        token.type = TOKEN_PUNCTUATION;
        token.value = ",";
        consumeChar();
        return token;
    }
    if (ch == ';') { // For for-loops
        token.type = TOKEN_PUNCTUATION;
        token.value = ";";
        consumeChar();
        return token;
    }
    if (ch == '<') {
        token.type = TOKEN_PUNCTUATION;
        consumeChar();
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
        consumeChar();
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
        consumeChar();
        if (content.at(lexingIndex) == '=') {
            consumeChar();
            token.value = "!=";
        } else {
            token.value = "!";
        }
        return token;
    }
    if (ch == '.') {
        token.type = TOKEN_PUNCTUATION;
        consumeChar();
        token.value = '.';
        return token;
    }
    if (ch == '"') {
        token.type = TOKEN_STRING;
        consumeChar();
        string val;
        while (lexingIndex < content.size() && content.at(lexingIndex) != '"') {
            // If char is an escape char
            if (content.at(lexingIndex) == '\\' && content.at(lexingIndex + 1) != '\\') {
                consumeChar(); // Consume '\'
                if (content.at(lexingIndex) == 'n') {
                    val += '\n';
                    consumeChar();
                } else if (content.at(lexingIndex) == 't') {
                    val += '\t';
                    consumeChar();
                } else if (content.at(lexingIndex) == '"') {
                    val += '"';
                    consumeChar();
                } else {
                    cerr << "Error on line " << column << ": unknown escape character " << '\\' << content.at(lexingIndex) << ". Treating character as unescaped" << endl;
                    val += '\\';
                    val += content.at(lexingIndex);
                    consumeChar();
                }
                continue;
            }
            val += content.at(lexingIndex);
            consumeChar();
        }
        // Consume '"'
        consumeChar();
        token.value = val;
        return token;
    }
    string word;
    if (isdigit(content.at(lexingIndex))) {
        token.type = TOKEN_NUMBER;
        while (lexingIndex < content.size() && isdigit(ch)) {
            word += (char) ch;
            consumeChar();
            if (lexingIndex >= content.size()) {
                break;
            }
            ch = content.at(lexingIndex);
        }
        token.value = word;
        if (content.at(lexingIndex) == '.') {
            token.value += '.';
            consumeChar();
            while (isdigit(content.at(lexingIndex))) {
                token.value += content.at(lexingIndex);
                consumeChar();
            }
        }
        if (content.at(lexingIndex) == 'f' || content.at(lexingIndex) == 'd') {
            token.value += content.at(lexingIndex);
            consumeChar();
        } else if (content.at(lexingIndex) == 'i') {
            token.value += content.at(lexingIndex);
            consumeChar();
            while (isdigit(content.at(lexingIndex))) {
                token.value += content.at(lexingIndex);
                consumeChar();
            }
        }
        return token;
    }

    while (lexingIndex < content.size() && (isalnum(ch) || ch == '_')) {
        word += (char) ch;
        consumeChar();
        if (lexingIndex >= content.size()) {
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
    if (word == "c") {
        token.type = TOKEN_CLASS;
        token.value = "";
        return token;
    }
    if (word == "n") {
        token.type = TOKEN_NEW;
        token.value = "";
        return token;
    }
    if (word == "m") {
        token.type = TOKEN_IMPORT;
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

// todo: for preprocessor: run lexer first, then find imports, then call tokenize with the data of the file and the name of the file (to include file names in error msgs), import files, repeat
vector<Token> tokenize(string data) {
    lexingIndex = 0;
    content = move(data);
    vector<Token> tokens;
    while (lexingIndex < content.size()) {
        struct Token t = readToken();
        cout << t.type << ":" << t.value << " @ [" << t.row + 1 << ":" << t.column + 1 << "]" << endl;
        tokens.push_back(t);
        if (lexingIndex < content.size() && content.at(lexingIndex) == '\n') {
            cout << TOKEN_NEWLINE << ":" << "" << " @ [" << row + 1 << ":" << column + 1 << "]" << endl;
            tokens.push_back({ TOKEN_NEWLINE, "", row, column, filePath });
            lexingIndex++;
            row++;
            column = 0;
        }
        while (lexingIndex < content.size() && (content.at(lexingIndex) == ' ' || content.at(lexingIndex) == '\n' || content.at(lexingIndex) == '\t')) {
            if (content.at(lexingIndex) == '\n') {
                row++;
                column = 0;
            } else {
                column++;
            }
            lexingIndex++;
        }
    }
    return tokens;
}

vector<Token> tokenizeFile(const string& fileName) {
    filePath = fileName;
    row = 0;
    column = 0;
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
