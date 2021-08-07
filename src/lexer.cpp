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
            consumeChar();
            token.type = TOKEN_PUNCTUATION;
            token.value = "/";
            return token;
        }
        if (content.at(lexingIndex + 1) == '/') {
            consumeChar(); // Consume first '/'
            consumeChar(); // Consume second '/'
            while (lexingIndex < content.size() && content.at(lexingIndex) != '\n' && content.at(lexingIndex) != EOF) {
                consumeChar();
            }
            if (lexingIndex < content.size()) {
                consumeChar(); // Consume newline
            }
            token.type = TOKEN_NEWLINE;
            row++;
            column = 0;
            return token;
        }
    }

    if (ch == '=') {
        token.type = TOKEN_PUNCTUATION;
        consumeChar();
        if (lexingIndex < content.size() && content.at(lexingIndex) == '=') {
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
        consumeChar();
        if (lexingIndex < content.size() && content.at(lexingIndex) == '=') {
            token.value = "+=";
            consumeChar();
        } else if (lexingIndex < content.size() && content.at(lexingIndex) == '+') {
            token.value = "++";
            consumeChar();
        } else {
            token.value = "+";
        }
        return token;
    }
    if (ch == '-') {
        token.type = TOKEN_PUNCTUATION;
        consumeChar();
        if (lexingIndex < content.size() && content.at(lexingIndex) == '=') {
            token.value = "-=";
            consumeChar();
        } else if (lexingIndex < content.size() && content.at(lexingIndex) == '-') {
            token.value = "--";
            consumeChar();
        } else {
            token.value = "-";
        }
        return token;
    }
    if (ch == '*') {
        token.type = TOKEN_PUNCTUATION;
        consumeChar();
        if (lexingIndex < content.size() && content.at(lexingIndex) == '=') {
            token.value = "*=";
            consumeChar();
        } else {
            token.value = "*";
        }
        return token;
    }
    if (ch == '/') {
        token.type = TOKEN_PUNCTUATION;
        consumeChar();
        if (lexingIndex < content.size() && content.at(lexingIndex) == '=') {
            token.value = "/=";
            consumeChar();
        } else {
            token.value = "/";
        }
        return token;
    }
    if (ch == '%') {
        token.type = TOKEN_PUNCTUATION;
        token.value = "%";
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
        if (lexingIndex < content.size() && content.at(lexingIndex) == '=') {
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
        if (lexingIndex < content.size() && content.at(lexingIndex) == '=') {
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
        if (lexingIndex < content.size() && content.at(lexingIndex) == '=') {
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
    if (ch == '?') {
        token.type = TOKEN_PUNCTUATION;
        consumeChar();
        token.value = '?';
        return token;
    }
    if (ch == '"') {
        token.type = TOKEN_STRING;
        consumeChar();
        string val;
        while (lexingIndex < content.size() && content.at(lexingIndex) != '"') {
            // If char is an escape char
            if (content.at(lexingIndex) == '\\') {
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
                } else if (content.at(lexingIndex) == '\\') {
                    val += '\\';
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

    if (ch == '|') {
        if (++lexingIndex >= content.size()) {
            cerr << "Error on line " << column << ": expected '|' after '|' to make \"||\" but instead found no more tokens" << endl;
            exit(EXIT_FAILURE);
        }
        if (content.at(lexingIndex) == '|') {
            consumeChar();
            token.type = TOKEN_PUNCTUATION;
            token.value = "||";
            return token;
        } else {
            // todo: bitwise and, or, not
            cerr << "Error on line " << column << ": expected '|' after '|' to make \"||\" but instead found " << content.at(lexingIndex) << endl;
            exit(EXIT_FAILURE);
        }
    }

    if (ch == '&') {
        if (++lexingIndex >= content.size()) {
            cerr << "Error on line " << column << ": expected '&' after '&' to make \"&&\" but instead found no more tokens" << endl;
            exit(EXIT_FAILURE);
        }
        if (content.at(lexingIndex) == '&') {
            consumeChar();
            token.type = TOKEN_PUNCTUATION;
            token.value = "&&";
            return token;
        } else {
            cerr << "Error on line " << column << ": expected '&' after '&' to make \"&&\" but instead found " << content.at(lexingIndex) << endl;
            exit(EXIT_FAILURE);
        }
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
        if (lexingIndex == content.size()) {
            return token;
        }
        if (content.at(lexingIndex) == '.') {
            token.value += '.';
            consumeChar();
            while (lexingIndex < content.size() && isdigit(content.at(lexingIndex))) {
                token.value += content.at(lexingIndex);
                consumeChar();
            }
        }
        if (lexingIndex >= content.size()) {
            return token;
        }
        if (content.at(lexingIndex) == 'i' || content.at(lexingIndex) == 'f' || content.at(lexingIndex) == 'd') {
            token.value += content.at(lexingIndex);
            consumeChar();
            while (lexingIndex < content.size() && isdigit(content.at(lexingIndex))) {
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
    if (word == "ret") {
        token.type = TOKEN_RETURN;
        token.value = "";
        return token;
    }
    if (word == "extern") {
        token.type = TOKEN_EXTERN;
        token.value = "";
        return token;
    }
    if (word == "class") {
        token.type = TOKEN_CLASS;
        token.value = "";
        return token;
    }
    if (word == "new") {
        token.type = TOKEN_NEW;
        token.value = "";
        return token;
    }
    if (word == "import") {
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
    if (word == "for") {
        token.type = TOKEN_FOR;
        token.value = "";
        return token;
    }
    if (word == "while") {
        token.type = TOKEN_WHILE;
        token.value = "";
        return token;
    }
    if (word == "extends") {
        token.type = TOKEN_EXTENDS;
        token.value = "";
        return token;
    }
    if (word == "virtual") {
        token.type = TOKEN_VIRTUAL;
        token.value = "";
        return token;
    }
    if (word == "override") {
        token.type = TOKEN_OVERRIDE;
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
