//
// Created by zack on 7/1/21.
//

#include <vector>
#include "LexerToken.h"
#include "llvm/IR/Constant.h"

#ifndef STELLAR_PARSER_H
#define STELLAR_PARSER_H

class ASTNode {
public:
    ~ASTNode() = default;
    virtual std::string toString() {
        return "ROOT_NODE";
    }
    virtual llvm::Value *codegen() = 0;
};

std::vector<ASTNode*> parse(std::vector<Token> tokens);

#endif //STELLAR_PARSER_H
