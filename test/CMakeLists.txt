
# fetch googletest
FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/7c07a863693b0c831f80473f7c6905d7e458682c.zip
)
FetchContent_MakeAvailable(googletest)
set(TESTSOURCES
  grammartests.cpp
  firstfollowtests.cpp
  tablegenerationtests.cpp
)
add_executable(fftest ${TESTSOURCES})
target_compile_definitions(fftest PUBLIC EXAMPLE_GRAMMAR_DIR="${CMAKE_SOURCE_DIR}/grammars/")
target_link_libraries(fftest PRIVATE jacc GTest::gtest_main)
include(GoogleTest)
gtest_discover_tests(fftest)

