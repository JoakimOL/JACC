cmake_minimum_required(VERSION 3.10)
project(firstfollow CXX)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(spdlog REQUIRED)
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

# fetch googletest
FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/7c07a863693b0c831f80473f7c6905d7e458682c.zip
)

FetchContent_MakeAvailable(googletest)

message(${CMAKE_CURRENT_BINARY_DIR})
message(${CMAKE_CURRENT_SOURCE_DIR})
set(COMMONSOURCES
    src/grammar.cpp
    src/firstfollow.cpp
)
set(MAINSOURCES
    src/main.cpp
    src/driver.cpp
    ${COMMONSOURCES}
)
set(TESTSOURCES
    test/grammartests.cpp
    ${COMMONSOURCES}
)

add_custom_target(generate-directory ALL
    COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_CURRENT_BINARY_DIR}/generated
)

enable_testing()

add_executable(ffgenerator ${MAINSOURCES} ${BISON_GrammarParser_OUTPUTS} ${FLEX_GrammarLexer_OUTPUTS})
add_executable(fftest ${TESTSOURCES})

set_target_properties(ffgenerator PROPERTIES CXX_STANDARD 17)
set_target_properties(fftest PROPERTIES CXX_STANDARD 17)

target_include_directories(
    ffgenerator PUBLIC
    ${CMAKE_CURRENT_BINARY_DIR}/generated
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)
target_include_directories(
    fftest PUBLIC
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
