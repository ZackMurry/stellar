cmake_minimum_required(VERSION 3.3)

project(test)

include(CTest)
enable_testing()

#set(gtest_disable_pthreads on) #needed in MinGW
include(DownloadProject.cmake)
download_project(
        PROJ                googletest
        GIT_REPOSITORY      https://github.com/google/googletest.git
        GIT_TAG             master
        UPDATE_DISCONNECTED 1
)


add_subdirectory(${googletest_SOURCE_DIR} ${googletest_BINARY_DIR} EXCLUDE_FROM_ALL)

add_executable(runUnitTests gtest.cpp lexer_test.cpp)
target_include_directories (
        runUnitTests
        PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/src
        ${CMAKE_CURRENT_SOURCE_DIR}/../src/include
        ${CMAKE_CURRENT_SOURCE_DIR}/../src
)
target_link_libraries(runUnitTests gtest gmock core)
