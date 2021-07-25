# MIT License
#
# Simple compiler front-end for LLVM written as a learning exercise.
# Copyright © 2017 Alberto Taiuti
#
# Permission is hereby granted, free of charge, to any person obtaining
# a copy of this software and associated documentation files (the "Software"),
# to deal in the Software without restriction, including without limitation
# the rights to use, copy, modify, merge, publish, distribute, sublicense,
# and/or sell copies of the Software, and to permit persons to whom the
# Software is furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included
# in all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
# EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
# OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
# IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
# DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
# TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE
# OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

cmake_minimum_required(VERSION 3.3)

project(stellar)

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    message(STATUS "Build mode: debug")
else()
    message(STATUS "Build mode: release")
endif (CMAKE_BUILD_TYPE STREQUAL "Debug")

# Export commands for YCM
set(CMAKE_EXPORT_COMPILE_COMMANDS 1)

# Set custom cmake modules path
set (CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules")

# Set default cmake build type
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug CACHE STRING
            "Choose the type of build, options are: None Debug Release RelWithDebInfo\
MinSizeRel. Default is Debug."
            FORCE)
endif(NOT CMAKE_BUILD_TYPE)

if(MSVC)
    # Force to always compile with W4
    if(CMAKE_CXX_FLAGS_DEBUG MATCHES "/W[0-4]")
        string(REGEX REPLACE "/W[0-4]" "/W4" CMAKE_CXX_FLAGS_DEBUG
                "${CMAKE_CXX_FLAGS_DEBUG}")
    else()
        set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /W4")
    endif()
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR
        CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    # Update if necessary
    set(CMAKE_CXX_FLAGS
            "${CMAKE_CXX_FLAGS} -std=c++14 -Wall -Wextra -fno-rtti")
    set(CMAKE_CXX_FLAGS_DEBUG  "${CMAKE_CXX_FLAGS_DEBUG} -O0")
    set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -O0")
    set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} -O2 -g")
    set(CMAKE_C_FLAGS_RELWITHDEBINFO "${CMAKE_C_FLAGS_RELWITHDEBINFO} -O2 -g")
endif()

if(UNIX)
    if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
        # Color error output
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fcolor-diagnostics")
    endif()
endif()

message(STATUS "Compiler custom flags are: ${CMAKE_CXX_FLAGS}")
message(STATUS "Compiler debug flags are: ${CMAKE_CXX_FLAGS_DEBUG}")
message(STATUS "Compiler release flags are: ${CMAKE_CXX_FLAGS_RELEASE}")

# Find and use LLVM
find_package(LLVM REQUIRED CONFIG)
message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")

include_directories(${LLVM_INCLUDE_DIRS})
add_definitions(${LLVM_DEFINITIONS})
message(STATUS ${LLVM_DEFINITIONS})
# Find the libraries that correspond to the LLVM components
# that we wish to use
llvm_map_components_to_libnames(LLVM_LIBS core support demangle mcjit native)
message(STATUS ${LLVM_LIBS})

# Create the main program
include_directories(
        inc
        src)
set(STELLAR_SRCS src/main.cpp src/lexer.cpp src/include/lexer.h src/include/LexerToken.h src/parser.cpp src/include/parser.h src/ASTVariableDefinition.cpp src/include/ASTVariableDefinition.h src/ASTVariableDeclaration.cpp src/include/ASTVariableDeclaration.h src/ASTBinaryExpression.cpp src/include/ASTBinaryExpression.h src/ASTVariableAssignment.cpp src/include/ASTVariableAssignment.h src/ASTVariableExpression.cpp src/include/ASTVariableExpression.h src/ASTNumberExpression.cpp src/include/ASTNumberExpression.h src/ASTFunctionDefinition.cpp src/include/ASTFunctionDefinition.h src/ASTFunctionInvocation.cpp src/include/ASTFunctionInvocation.h src/ASTArrayDefinition.cpp src/include/ASTArrayDefinition.h src/ASTArrayAccess.cpp src/include/ASTArrayAccess.h src/ASTIfStatement.cpp src/include/ASTIfStatement.h src/ASTArrayIndexAssignment.cpp src/include/ASTArrayIndexAssignment.h src/ASTReturn.cpp src/include/ASTReturn.h src/ASTExternDeclaration.cpp src/include/ASTExternDeclaration.h src/ASTStringExpression.cpp src/include/ASTStringExpression.h src/ASTClassDefinition.cpp src/include/ASTClassDefinition.h src/ASTClassInstantiation.cpp src/include/ASTClassInstantiation.h src/ASTClassFieldAccess.cpp src/include/ASTClassFieldAccess.h src/ASTClassFieldStore.cpp src/include/ASTClassFieldStore.h src/ASTMethodCall.cpp src/include/ASTMethodCall.h src/ASTNewExpression.cpp src/include/ASTNewExpression.h src/preprocessor.cpp src/include/preprocessor.h src/codegen.cpp src/include/codegen.h src/ASTForExpression.cpp src/include/ASTForExpression.h src/ASTWhileExpression.cpp src/include/ASTWhileExpression.h)

add_definitions(-D_STELLAR_DEBUG_LVL_2)


link_libraries()
execute_process(COMMAND llvm-config --libs OUTPUT_VARIABLE LIBS)
execute_process(COMMAND llvm-config --system-libs OUTPUT_VARIABLE SYS_LIBS)
execute_process(COMMAND llvm-config --ldflags OUTPUT_VARIABLE LDF)
#message(STATUS "Found LLVM" ${LIBS})

string(STRIP ${LIBS} LIBS)
string(STRIP ${SYS_LIBS} SYS_LIBS)
string(STRIP ${LDF} LDF)

link_libraries(${LIBS} ${SYS_LIBS} ${LDF})

execute_process(COMMAND llvm-config --cxxflags OUTPUT_VARIABLE CMAKE_CXX_FLAGS)
string(STRIP ${CMAKE_CXX_FLAGS} CMAKE_CXX_FLAGS)

add_library(core ${STELLAR_SRCS})
add_executable(exe src/main.cpp)
target_link_libraries(exe core)
target_link_libraries(exe
        ${LLVM_LIBS})
target_include_directories (
        exe
        PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/src/include
)

# Uncomment for tests
include(CTest)
enable_testing()

if(BUILD_TESTING)
    add_subdirectory (test)
endif()
