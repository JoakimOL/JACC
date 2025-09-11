cmake_minimum_required(VERSION 3.10)
project(firstfollow CXX)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(BISON REQUIRED)
find_package(FLEX REQUIRED)

BISON_TARGET(GrammarParser src/parser.yy ${CMAKE_CURRENT_BINARY_DIR}/generated/parser.cpp
    DEFINES_FILE ${CMAKE_CURRENT_BINARY_DIR}/generated/parser.h)

FLEX_TARGET(GrammarLexer src/lexer.ll ${CMAKE_CURRENT_BINARY_DIR}/generated/lexer.cpp
    DEFINES_FILE ${CMAKE_CURRENT_BINARY_DIR}/generated/lexer.h)

ADD_FLEX_BISON_DEPENDENCY(GrammarLexer GrammarParser)

# fetch argparse
include(FetchContent)
FetchContent_Declare(
    argparse
    GIT_REPOSITORY https://github.com/p-ranav/argparse.git
    GIT_TAG af442b4
)
FetchContent_MakeAvailable(argparse)

FetchContent_Declare(
  fmt
  GIT_REPOSITORY https://github.com/fmtlib/fmt
  GIT_TAG        11.2.0)
FetchContent_MakeAvailable(fmt)

FetchContent_Declare(
  spdlog
  GIT_REPOSITORY https://github.com/gabime/spdlog
  GIT_TAG        v1.15.3) 
FetchContent_MakeAvailable(spdlog)

# fetch googletest
FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/7c07a863693b0c831f80473f7c6905d7e458682c.zip
)

FetchContent_MakeAvailable(googletest)

message(${CMAKE_CURRENT_BINARY_DIR})
message(${CMAKE_CURRENT_SOURCE_DIR})
set(COMMONSOURCES
    src/driver.cpp
    src/first_follow_set_generator.cpp
    src/grammar.cpp
    src/ll_table_generator.cpp
    src/table_driven_ll_parser.cpp
)
set(MAINSOURCES
    src/main.cpp
    ${COMMONSOURCES}
)
set(TESTSOURCES
    test/grammartests.cpp
    test/firstfollowtests.cpp
    test/tablegenerationtests.cpp
    ${COMMONSOURCES}
)

add_custom_target(generate-directory ALL
    COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_CURRENT_BINARY_DIR}/generated
)

enable_testing()

add_executable(ffgenerator ${MAINSOURCES} ${BISON_GrammarParser_OUTPUTS} ${FLEX_GrammarLexer_OUTPUTS})
add_executable(fftest ${TESTSOURCES} ${BISON_GrammarParser_OUTPUTS} ${FLEX_GrammarLexer_OUTPUTS})

target_compile_definitions(fftest PUBLIC EXAMPLE_GRAMMAR_DIR="${CMAKE_CURRENT_SOURCE_DIR}/grammars/")


set_target_properties(ffgenerator PROPERTIES CXX_STANDARD 20)
set_target_properties(fftest PROPERTIES CXX_STANDARD 20)

target_include_directories(
    ffgenerator PUBLIC
    ${CMAKE_CURRENT_BINARY_DIR}/generated
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)
target_include_directories(
    fftest PUBLIC
    ${CMAKE_CURRENT_BINARY_DIR}/generated
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_compile_options(ffgenerator PRIVATE
  -Wall
  -Werror
  -Wconversion
  -Wunused-variable
  -pedantic
)

target_link_libraries(ffgenerator PRIVATE
    spdlog::spdlog
    argparse
)

target_link_libraries(fftest PRIVATE
    spdlog::spdlog
    argparse
    GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(fftest)
