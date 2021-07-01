//
// Created by zack on 7/1/21.
//
#include "gtest/gtest.h"
#include "include/lexer.h"
#include <vector>

using namespace std;

TEST(LexerTest, TestTest) {
    vector<Token> tokens = tokenize("i32 myVal = 2");

    ASSERT_EQ(tokens.size(), 4);

    ASSERT_EQ(tokens[0].type, TOKEN_IDENTIFIER);
    ASSERT_EQ(tokens[0].value, "i32");

    ASSERT_EQ(tokens[1].type, TOKEN_IDENTIFIER);
    ASSERT_EQ(tokens[1].value, "myVal");

    ASSERT_EQ(tokens[2].type, TOKEN_PUNCTUATION);
    ASSERT_EQ(tokens[2].value, "=");

    ASSERT_EQ(tokens[3].type, TOKEN_NUMBER);
    ASSERT_EQ(tokens[3].value, "2");
}
