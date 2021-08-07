//
// Created by zack on 7/30/21.
//

#include "gtest/gtest.h"
#include "include/lexer.h"
#include "include/parser.h"
#include <vector>

using namespace std;

TEST(ParserTest, IntegerDeclarationTest) {
    auto nodes = parse(tokenize("i32 i = 0"));

    ASSERT_EQ(nodes.size(), 1);
    ASSERT_EQ(nodes[0]->toString(), "[VAR_DECL: i i32 [NUMBER: 0 type: 2]]");
}

TEST(ParserTest, FloatDeclarationTest) {
    auto nodes = parse(tokenize("f32 myFloat = 2.0"));

    ASSERT_EQ(nodes.size(), 1);
    ASSERT_EQ(nodes[0]->toString(), "[VAR_DECL: myFloat f32 [NUMBER: 2.0 type: 4]]");
}

TEST(ParserTest, DoubleDeclarationTest) {
    auto nodes = parse(tokenize("f64 myDouble = 2.0f64"));

    ASSERT_EQ(nodes.size(), 1);
    ASSERT_EQ(nodes[0]->toString(), "[VAR_DECL: myDouble f64 [NUMBER: 2.0 type: 5]]");
}

TEST(ParserTest, FunctionDefinitionTest) {
    auto nodes = parse(tokenize("void myFunc() {}"));
    ASSERT_EQ(nodes.size(), 1);
    ASSERT_EQ(nodes[0]->toString(), "[FUN_DEF: myFunc args: [] body: [] returnType: void]");

    nodes = parse(tokenize("i32 myFunc() {}"));
    ASSERT_EQ(nodes.size(), 1);
    ASSERT_EQ(nodes[0]->toString(), "[FUN_DEF: myFunc args: [] body: [] returnType: i32]");

    nodes = parse(tokenize("void myFunc() {i32 i = 0}"));
    ASSERT_EQ(nodes.size(), 1);
    ASSERT_EQ(nodes[0]->toString(), "[FUN_DEF: myFunc args: [] body: [[VAR_DECL: i i32 [NUMBER: 0 type: 2]]] returnType: void]");

    nodes = parse(tokenize("void myFunc() {i32 i = 0\ni32 j = 1\ni32 k = 2}"));
    ASSERT_EQ(nodes.size(), 1);
    ASSERT_EQ(nodes[0]->toString(), "[FUN_DEF: myFunc args: [] body: [[VAR_DECL: i i32 [NUMBER: 0 type: 2]][VAR_DECL: j i32 [NUMBER: 1 type: 2]][VAR_DECL: k i32 [NUMBER: 2 type: 2]]] returnType: void]");

    nodes = parse(tokenize("void myFunc(i32 arg) {}"));
    ASSERT_EQ(nodes.size(), 1);
    ASSERT_EQ(nodes[0]->toString(), "[FUN_DEF: myFunc args: [[VAR_DEF: arg i32]] body: [] returnType: void]");

    nodes = parse(tokenize("void myFunc(i32 arg1, i32 arg2) {}"));
    ASSERT_EQ(nodes.size(), 1);
    ASSERT_EQ(nodes[0]->toString(), "[FUN_DEF: myFunc args: [[VAR_DEF: arg1 i32][VAR_DEF: arg2 i32]] body: [] returnType: void]");

    nodes = parse(tokenize("void myFunc(i8 arg1, i16 arg2, i32 arg3, i64 arg4) {}"));
    ASSERT_EQ(nodes.size(), 1);
    ASSERT_EQ(nodes[0]->toString(), "[FUN_DEF: myFunc args: [[VAR_DEF: arg1 i8][VAR_DEF: arg2 i16][VAR_DEF: arg3 i32][VAR_DEF: arg4 i64]] body: [] returnType: void]");
}

TEST(ParserTest, FunctionInvocationTest) {
    auto nodes = parse(tokenize("printf()"));
    ASSERT_EQ(nodes.size(), 1);
    ASSERT_EQ(nodes[0]->toString(), "[FUN_INV: printf args: []]");

    nodes = parse(tokenize(R"(printf("Hello, world!\n"))"));
    ASSERT_EQ(nodes.size(), 1);
    ASSERT_EQ(nodes[0]->toString(), "[FUN_INV: printf args: [[STRING: Hello, world!\n]]]");

    nodes = parse(tokenize(R"(printf("Here is a number: %d\n", 2))"));
    ASSERT_EQ(nodes.size(), 1);
    ASSERT_EQ(nodes[0]->toString(), "[FUN_INV: printf args: [[STRING: Here is a number: %d\n][NUMBER: 2 type: 2]]]");
}

TEST(ParserTest, ArrayDefinitionTest) {
    auto nodes = parse(tokenize("i32[5] nums"));
    ASSERT_EQ(nodes.size(), 1);
    ASSERT_EQ(nodes[0]->toString(), "[ARR_DEF: nums i32 size: [NUMBER: 5 type: 2]]");

    nodes = parse(tokenize("i32[length] myArr"));
    ASSERT_EQ(nodes.size(), 1);
    ASSERT_EQ(nodes[0]->toString(), "[ARR_DEF: myArr i32 size: [VARIABLE: length]]");

    nodes = parse(tokenize("i32[1 + 2] myArr"));
    ASSERT_EQ(nodes.size(), 1);
    ASSERT_EQ(nodes[0]->toString(), "[ARR_DEF: myArr i32 size: [BIN_EXP: 0 [NUMBER: 1 type: 2] [NUMBER: 2 type: 2]]]");
}

TEST(ParserTest, ArrayAccessTest) {
    auto nodes = parse(tokenize("i32 val = myArr[4]"));
    ASSERT_EQ(nodes.size(), 1);
    ASSERT_EQ(nodes[0]->toString(), "[VAR_DECL: val i32 [ARR_ACCESS: myArr at [NUMBER: 4 type: 2]]]");

    nodes = parse(tokenize("i32 val = myArr[1+2]"));
    ASSERT_EQ(nodes.size(), 1);
    ASSERT_EQ(nodes[0]->toString(), "[VAR_DECL: val i32 [ARR_ACCESS: myArr at [BIN_EXP: 0 [NUMBER: 1 type: 2] [NUMBER: 2 type: 2]]]]");

    nodes = parse(tokenize("i32 val = myArr[index]"));
    ASSERT_EQ(nodes.size(), 1);
    ASSERT_EQ(nodes[0]->toString(), "[VAR_DECL: val i32 [ARR_ACCESS: myArr at [VARIABLE: index]]]");
}

TEST(ParserTest, ArrayIndexAssignmentTest) {
    auto nodes = parse(tokenize("myArr[0] = 2"));
    ASSERT_EQ(nodes.size(), 1);
    ASSERT_EQ(nodes[0]->toString(), "[ARR_ASSIGN: myArr at [NUMBER: 0 type: 2] to [NUMBER: 2 type: 2]]");

    nodes = parse(tokenize("myArr[1+2] = 2"));
    ASSERT_EQ(nodes.size(), 1);
    ASSERT_EQ(nodes[0]->toString(), "[ARR_ASSIGN: myArr at [BIN_EXP: 0 [NUMBER: 1 type: 2] [NUMBER: 2 type: 2]] to [NUMBER: 2 type: 2]]");

    nodes = parse(tokenize("myArr[index] = val"));
    ASSERT_EQ(nodes.size(), 1);
    ASSERT_EQ(nodes[0]->toString(), "[ARR_ASSIGN: myArr at [VARIABLE: index] to [VARIABLE: val]]");
}
