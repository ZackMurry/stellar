#include <iostream>
#include <vector>
#include "include/lexer.h"
#include "include/parser.h"

using namespace std;

int main(int argc, char *argv[]) {
    if (argc < 2) {
        cerr << "Error: expected input file" << endl;
        exit(EXIT_FAILURE);
    }
    vector<Token> tokens = tokenizeFile(argv[1]);
    vector<ASTNode*> nodes = parse(tokens);
    return 0;
}
