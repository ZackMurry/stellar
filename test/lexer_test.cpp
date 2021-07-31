//
// Created by zack on 7/1/21.
//
#include "gtest/gtest.h"
#include "include/lexer.h"
#include <vector>

using namespace std;

TEST(LexerTest, IntegerDeclarationTest) {
    auto tokens = tokenize("i32 myVal = 2");

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

TEST(LexerTest, FloatDeclarationTest) {
    auto tokens = tokenize("f myFloat = 2.0");

    ASSERT_EQ(tokens.size(), 4);

    ASSERT_EQ(tokens[0].type, TOKEN_IDENTIFIER);
    ASSERT_EQ(tokens[0].value, "f");

    ASSERT_EQ(tokens[1].type, TOKEN_IDENTIFIER);
    ASSERT_EQ(tokens[1].value, "myFloat");

    ASSERT_EQ(tokens[2].type, TOKEN_PUNCTUATION);
    ASSERT_EQ(tokens[2].value, "=");

    ASSERT_EQ(tokens[3].type, TOKEN_NUMBER);
    ASSERT_EQ(tokens[3].value, "2.0");
}

TEST(LexerTest, DoubleDeclarationTest) {
    auto tokens = tokenize("d myDouble = 2.0d");

    ASSERT_EQ(tokens.size(), 4);

    ASSERT_EQ(tokens[0].type, TOKEN_IDENTIFIER);
    ASSERT_EQ(tokens[0].value, "d");

    ASSERT_EQ(tokens[1].type, TOKEN_IDENTIFIER);
    ASSERT_EQ(tokens[1].value, "myDouble");

    ASSERT_EQ(tokens[2].type, TOKEN_PUNCTUATION);
    ASSERT_EQ(tokens[2].value, "=");

    ASSERT_EQ(tokens[3].type, TOKEN_NUMBER);
    ASSERT_EQ(tokens[3].value, "2.0d");
}

TEST(LexerTest, FloatTest) {
    auto tokens = tokenize("3f");

    ASSERT_EQ(tokens.size(), 1);
    ASSERT_EQ(tokens[0].type, TOKEN_NUMBER);
    ASSERT_EQ(tokens[0].value, "3f");

    tokens = tokenize("13.37");
    ASSERT_EQ(tokens.size(), 1);
    ASSERT_EQ(tokens[0].type, TOKEN_NUMBER);
    ASSERT_EQ(tokens[0].value, "13.37");
}

TEST(LexerTest, DoubleTest) {
    auto tokens = tokenize("3d");

    ASSERT_EQ(tokens.size(), 1);
    ASSERT_EQ(tokens[0].type, TOKEN_NUMBER);
    ASSERT_EQ(tokens[0].value, "3d");

    tokens = tokenize("13.37d");
    ASSERT_EQ(tokens.size(), 1);
    ASSERT_EQ(tokens[0].type, TOKEN_NUMBER);
    ASSERT_EQ(tokens[0].value, "13.37d");
}

TEST(LexerTest, IntegerTest) {
    auto tokens = tokenize("3i8");

    ASSERT_EQ(tokens.size(), 1);
    ASSERT_EQ(tokens[0].type, TOKEN_NUMBER);
    ASSERT_EQ(tokens[0].value, "3i8");

    tokens = tokenize("4i16");
    ASSERT_EQ(tokens.size(), 1);
    ASSERT_EQ(tokens[0].type, TOKEN_NUMBER);
    ASSERT_EQ(tokens[0].value, "4i16");

    tokens = tokenize("32132234i32");
    ASSERT_EQ(tokens.size(), 1);
    ASSERT_EQ(tokens[0].type, TOKEN_NUMBER);
    ASSERT_EQ(tokens[0].value, "32132234i32");

    tokens = tokenize("32132234");
    ASSERT_EQ(tokens.size(), 1);
    ASSERT_EQ(tokens[0].type, TOKEN_NUMBER);
    ASSERT_EQ(tokens[0].value, "32132234");

    tokens = tokenize("2313i64");
    ASSERT_EQ(tokens.size(), 1);
    ASSERT_EQ(tokens[0].type, TOKEN_NUMBER);
    ASSERT_EQ(tokens[0].value, "2313i64");
}

TEST(LexerTest, ClassTest) {
    auto tokens = tokenize("class");

    ASSERT_EQ(tokens.size(), 1);
    ASSERT_EQ(tokens[0].type, TOKEN_CLASS);
    ASSERT_EQ(tokens[0].value, "");

    tokens = tokenize("class MyClass");

    ASSERT_EQ(tokens.size(), 2);

    ASSERT_EQ(tokens[0].type, TOKEN_CLASS);
    ASSERT_EQ(tokens[0].value, "");

    ASSERT_EQ(tokens[1].type, TOKEN_IDENTIFIER);
    ASSERT_EQ(tokens[1].value, "MyClass");
}

TEST(LexerTest, PunctuationTest) {
    auto tokens = tokenize("=");
    ASSERT_EQ(tokens.size(), 1);
    ASSERT_EQ(tokens[0].type, TOKEN_PUNCTUATION);
    ASSERT_EQ(tokens[0].value, "=");

    tokens = tokenize("==");
    ASSERT_EQ(tokens.size(), 1);
    ASSERT_EQ(tokens[0].type, TOKEN_PUNCTUATION);
    ASSERT_EQ(tokens[0].value, "==");

    tokens = tokenize("(");
    ASSERT_EQ(tokens.size(), 1);
    ASSERT_EQ(tokens[0].type, TOKEN_PUNCTUATION);
    ASSERT_EQ(tokens[0].value, "(");

    tokens = tokenize(")");
    ASSERT_EQ(tokens.size(), 1);
    ASSERT_EQ(tokens[0].type, TOKEN_PUNCTUATION);
    ASSERT_EQ(tokens[0].value, ")");

    tokens = tokenize("{");
    ASSERT_EQ(tokens.size(), 1);
    ASSERT_EQ(tokens[0].type, TOKEN_PUNCTUATION);
    ASSERT_EQ(tokens[0].value, "{");

    tokens = tokenize("}");
    ASSERT_EQ(tokens.size(), 1);
    ASSERT_EQ(tokens[0].type, TOKEN_PUNCTUATION);
    ASSERT_EQ(tokens[0].value, "}");

    tokens = tokenize("[");
    ASSERT_EQ(tokens.size(), 1);
    ASSERT_EQ(tokens[0].type, TOKEN_PUNCTUATION);
    ASSERT_EQ(tokens[0].value, "[");

    tokens = tokenize("]");
    ASSERT_EQ(tokens.size(), 1);
    ASSERT_EQ(tokens[0].type, TOKEN_PUNCTUATION);
    ASSERT_EQ(tokens[0].value, "]");

    tokens = tokenize("+");
    ASSERT_EQ(tokens.size(), 1);
    ASSERT_EQ(tokens[0].type, TOKEN_PUNCTUATION);
    ASSERT_EQ(tokens[0].value, "+");

    tokens = tokenize("+=");
    ASSERT_EQ(tokens.size(), 1);
    ASSERT_EQ(tokens[0].type, TOKEN_PUNCTUATION);
    ASSERT_EQ(tokens[0].value, "+=");

    tokens = tokenize("++");
    ASSERT_EQ(tokens.size(), 1);
    ASSERT_EQ(tokens[0].type, TOKEN_PUNCTUATION);
    ASSERT_EQ(tokens[0].value, "++");

    tokens = tokenize("-");
    ASSERT_EQ(tokens.size(), 1);
    ASSERT_EQ(tokens[0].type, TOKEN_PUNCTUATION);
    ASSERT_EQ(tokens[0].value, "-");

    tokens = tokenize("-=");
    ASSERT_EQ(tokens.size(), 1);
    ASSERT_EQ(tokens[0].type, TOKEN_PUNCTUATION);
    ASSERT_EQ(tokens[0].value, "-=");

    tokens = tokenize("--");
    ASSERT_EQ(tokens.size(), 1);
    ASSERT_EQ(tokens[0].type, TOKEN_PUNCTUATION);
    ASSERT_EQ(tokens[0].value, "--");

    tokens = tokenize("*");
    ASSERT_EQ(tokens.size(), 1);
    ASSERT_EQ(tokens[0].type, TOKEN_PUNCTUATION);
    ASSERT_EQ(tokens[0].value, "*");

    tokens = tokenize("/");
    ASSERT_EQ(tokens.size(), 1);
    ASSERT_EQ(tokens[0].type, TOKEN_PUNCTUATION);
    ASSERT_EQ(tokens[0].value, "/");

    tokens = tokenize(",");
    ASSERT_EQ(tokens.size(), 1);
    ASSERT_EQ(tokens[0].type, TOKEN_PUNCTUATION);
    ASSERT_EQ(tokens[0].value, ",");

    tokens = tokenize(";");
    ASSERT_EQ(tokens.size(), 1);
    ASSERT_EQ(tokens[0].type, TOKEN_PUNCTUATION);
    ASSERT_EQ(tokens[0].value, ";");

    tokens = tokenize("<");
    ASSERT_EQ(tokens.size(), 1);
    ASSERT_EQ(tokens[0].type, TOKEN_PUNCTUATION);
    ASSERT_EQ(tokens[0].value, "<");

    tokens = tokenize("<=");
    ASSERT_EQ(tokens.size(), 1);
    ASSERT_EQ(tokens[0].type, TOKEN_PUNCTUATION);
    ASSERT_EQ(tokens[0].value, "<=");

    tokens = tokenize(">");
    ASSERT_EQ(tokens.size(), 1);
    ASSERT_EQ(tokens[0].type, TOKEN_PUNCTUATION);
    ASSERT_EQ(tokens[0].value, ">");

    tokens = tokenize(">=");
    ASSERT_EQ(tokens.size(), 1);
    ASSERT_EQ(tokens[0].type, TOKEN_PUNCTUATION);
    ASSERT_EQ(tokens[0].value, ">=");

    tokens = tokenize("!");
    ASSERT_EQ(tokens.size(), 1);
    ASSERT_EQ(tokens[0].type, TOKEN_PUNCTUATION);
    ASSERT_EQ(tokens[0].value, "!");

    tokens = tokenize("!=");
    ASSERT_EQ(tokens.size(), 1);
    ASSERT_EQ(tokens[0].type, TOKEN_PUNCTUATION);
    ASSERT_EQ(tokens[0].value, "!=");

    tokens = tokenize(".");
    ASSERT_EQ(tokens.size(), 1);
    ASSERT_EQ(tokens[0].type, TOKEN_PUNCTUATION);
    ASSERT_EQ(tokens[0].value, ".");
}

TEST(LexerTest, ExternTest) {
    auto tokens = tokenize("extern");
    ASSERT_EQ(tokens.size(), 1);
    ASSERT_EQ(tokens[0].type, TOKEN_EXTERN);
    ASSERT_EQ(tokens[0].value, "");
}

TEST(LexerTest, BooleanTest) {
    auto tokens = tokenize("true");
    ASSERT_EQ(tokens.size(), 1);
    ASSERT_EQ(tokens[0].type, TOKEN_BOOLEAN);
    ASSERT_EQ(tokens[0].value, "true");

    tokens = tokenize("false");
    ASSERT_EQ(tokens.size(), 1);
    ASSERT_EQ(tokens[0].type, TOKEN_BOOLEAN);
    ASSERT_EQ(tokens[0].value, "false");
}

TEST(LexerTest, ReturnTest) {
    auto tokens = tokenize("ret");
    ASSERT_EQ(tokens.size(), 1);
    ASSERT_EQ(tokens[0].type, TOKEN_RETURN);
    ASSERT_EQ(tokens[0].value, "");
}

TEST(LexerTest, NewTest) {
    auto tokens = tokenize("new");
    ASSERT_EQ(tokens.size(), 1);
    ASSERT_EQ(tokens[0].type, TOKEN_NEW);
    ASSERT_EQ(tokens[0].value, "");
}

TEST(LexerTest, ImportTest) {
    auto tokens = tokenize("import");
    ASSERT_EQ(tokens.size(), 1);
    ASSERT_EQ(tokens[0].type, TOKEN_IMPORT);
    ASSERT_EQ(tokens[0].value, "");
}

TEST(LexerTest, IfTest) {
    auto tokens = tokenize("if");
    ASSERT_EQ(tokens.size(), 1);
    ASSERT_EQ(tokens[0].type, TOKEN_IF);
    ASSERT_EQ(tokens[0].value, "");
}

TEST(LexerTest, ElseTest) {
    auto tokens = tokenize("else");
    ASSERT_EQ(tokens.size(), 1);
    ASSERT_EQ(tokens[0].type, TOKEN_ELSE);
    ASSERT_EQ(tokens[0].value, "");
}

TEST(LexerTest, ForTest) {
    auto tokens = tokenize("for");
    ASSERT_EQ(tokens.size(), 1);
    ASSERT_EQ(tokens[0].type, TOKEN_FOR);
    ASSERT_EQ(tokens[0].value, "");
}

TEST(LexerTest, WhileTest) {
    auto tokens = tokenize("while");
    ASSERT_EQ(tokens.size(), 1);
    ASSERT_EQ(tokens[0].type, TOKEN_WHILE);
    ASSERT_EQ(tokens[0].value, "");
}

TEST(LexerTest, StringTest) {
    auto tokens = tokenize("\"Hello, world!\"");
    ASSERT_EQ(tokens.size(), 1);
    ASSERT_EQ(tokens[0].type, TOKEN_STRING);
    ASSERT_EQ(tokens[0].value, "Hello, world!");

    tokens = tokenize(R"("Hello, world!\n")");
    ASSERT_EQ(tokens.size(), 1);
    ASSERT_EQ(tokens[0].type, TOKEN_STRING);
    ASSERT_EQ(tokens[0].value, "Hello, world!\n");

    tokens = tokenize(R"("\tHello, world!\n")");
    ASSERT_EQ(tokens.size(), 1);
    ASSERT_EQ(tokens[0].type, TOKEN_STRING);
    ASSERT_EQ(tokens[0].value, "\tHello, world!\n");

    tokens = tokenize(R"("\tHello, \"world!\n")");
    ASSERT_EQ(tokens.size(), 1);
    ASSERT_EQ(tokens[0].type, TOKEN_STRING);
    ASSERT_EQ(tokens[0].value, "\tHello, \"world!\n");

    tokens = tokenize(R"("\tHello, \"wo\\rld!\n")");
    ASSERT_EQ(tokens.size(), 1);
    ASSERT_EQ(tokens[0].type, TOKEN_STRING);
    ASSERT_EQ(tokens[0].value, "\tHello, \"wo\\rld!\n");
}
