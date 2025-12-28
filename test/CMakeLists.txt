
# fetch googletest
FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest    
    GIT_TAG b514bdc
)

FetchContent_MakeAvailable(googletest)
set(TESTSOURCES
  grammartests.cpp
  firstfollowtests.cpp
  tablegenerationtests.cpp
)
add_executable(fftest ${TESTSOURCES})
target_compile_definitions(fftest PUBLIC EXAMPLE_GRAMMAR_DIR="${CMAKE_SOURCE_DIR}/grammars/")
target_link_libraries(fftest PRIVATE jacc fmt::fmt spdlog::spdlog GTest::gtest_main)
include(GoogleTest)
gtest_discover_tests(fftest)

