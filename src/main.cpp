#include <iostream>
#include <vector>
#include "include/lexer.h"
#include "include/parser.h"
#include "include/preprocessor.h"

using namespace std;

// todo: a standard library
// argv[1] is the file to compile, argv[2] is the path to the std library
// todo: proper command line options
int main(int argc, char *argv[]) {
    if (argc < 2) {
        cerr << "Error: expected input file" << endl;
        exit(EXIT_FAILURE);
    }
    vector<Token> tokens = tokenizeFile(argv[1]);
    tokens = preprocessTokens(tokens, argv[1], argv[2]);
    cout << "Lexing complete" << endl;
    vector<ASTNode*> nodes = parse(tokens);
    return 0;
}
