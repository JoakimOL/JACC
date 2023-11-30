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
# pinned version because stability in my life
include(FetchContent)
FetchContent_Declare(
    argparse
    GIT_REPOSITORY https://github.com/p-ranav/argparse.git
    GIT_TAG af442b4
)
FetchContent_MakeAvailable(argparse)

message(${CMAKE_CURRENT_BINARY_DIR})
message(${CMAKE_CURRENT_SOURCE_DIR})
set(SOURCES
    src/main.cpp
    src/grammar.cpp
    src/driver.cpp
)

add_custom_target(generate-directory ALL
    COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_CURRENT_BINARY_DIR}/generated
)

add_executable(ffgenerator ${SOURCES} ${BISON_GrammarParser_OUTPUTS} ${FLEX_GrammarLexer_OUTPUTS})

set_target_properties(ffgenerator PROPERTIES CXX_STANDARD 17)

target_include_directories(
    ffgenerator PUBLIC
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
