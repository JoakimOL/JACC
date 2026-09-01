find_package(BISON REQUIRED)
find_package(FLEX REQUIRED)

BISON_TARGET(GrammarParser parser.yy ${CMAKE_CURRENT_BINARY_DIR}/generated/parser.cpp
    DEFINES_FILE ${CMAKE_CURRENT_BINARY_DIR}/generated/parser.h)

FLEX_TARGET(GrammarLexer lexer.ll ${CMAKE_CURRENT_BINARY_DIR}/generated/lexer.cpp
    DEFINES_FILE ${CMAKE_CURRENT_BINARY_DIR}/generated/lexer.h)

ADD_FLEX_BISON_DEPENDENCY(GrammarLexer GrammarParser)

add_library(jacc
    driver.cpp
    first_follow_set_generator.cpp
    grammar.cpp
    ll_table_generator.cpp
    table_driven_ll_parser.cpp
    bearpiglexer.cpp
    ${BISON_GrammarParser_OUTPUTS}
    ${FLEX_GrammarLexer_OUTPUTS}
)

target_include_directories(jacc
  PUBLIC
  ${jacc_SOURCE_DIR}/include/
  ${CMAKE_CURRENT_BINARY_DIR}/generated/ # jacc/build/src/generated
)

target_compile_options(jacc PRIVATE
  -Wall
  -Werror
  -Wconversion
  -Wunused-variable
  -pedantic
)

# Bison's generated parser declares yynerrs_ but only reads it when the
# grammar uses error recovery/verbose errors; newer GCC flags it as
# set-but-unused. Don't fail the build on this one warning in generated code.
set_source_files_properties(
  ${BISON_GrammarParser_OUTPUTS}
  PROPERTIES
  COMPILE_OPTIONS "-Wno-error=unused-but-set-variable"
)

# target_include_directories(jacc PUBLIC ../include)

target_link_libraries(jacc PUBLIC
  fmt::fmt
)
target_link_libraries(jacc PRIVATE
  spdlog::spdlog
  libbearpig
)

# All users of this library will need at least C++20
target_compile_features(jacc PUBLIC cxx_std_20)

