//
// Created by zack on 7/20/21.
//
#include "include/preprocessor.h"
#include "include/lexer.h"
#include <iostream>
#include <set>

set<string> importedFiles;

vector<Token> preprocessTokens(const vector<Token>& tokens, const string& filePath, const string& stdPath) {
    importedFiles.clear();
    importedFiles.insert(filePath);
    vector<Token> newTokens;
    for (int i = 0; i < tokens.size(); i++) {
        if (tokens.at(i).type != TOKEN_IMPORT) {
           newTokens.push_back(tokens.at(i));
           continue;
        }
        if (++i >= tokens.size()) {
            cerr << "Error: invalid import" << endl;
            exit(EXIT_FAILURE);
        }
        if (tokens.at(i).type != TOKEN_STRING) {
            cerr << "Error: 'm' should be followed by a string (indicating the file to import), but instead it was followed by token type " << tokens.at(i).type << " at " << to_string(tokens.at(i).row + 1) + ":" + to_string(tokens.at(i).column + 1) << endl;
            exit(EXIT_FAILURE);
        }
        string importPath = tokens.at(i).value;
        if (importedFiles.count(importPath)) {
            cout << importPath << " has already been imported. Skipping..." << endl;
            continue;
        }
        cout << "Import path: " << importPath << endl;
        vector<Token> importedContent;
        if (importPath.substr(0, 3) == "std") {
            cout << "Importing std file " << importPath.substr(3) << endl;
            importedContent = preprocessTokens(tokenizeFile(stdPath + importPath.substr(3)), filePath, stdPath);
        } else {
            importedContent = preprocessTokens(tokenizeFile(importPath), filePath, stdPath);
        }
        cout << "tokens imported: " << importedContent.size() << endl;
        for (const auto& it : importedContent) {
            newTokens.push_back(it);
        }
    }
    return newTokens;
}
