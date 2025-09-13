# fetch argparse
fetchcontent_declare(
    argparse
    GIT_REPOSITORY https://github.com/p-ranav/argparse.git
    GIT_TAG af442b4
)
fetchcontent_makeavailable(argparse)
add_executable(ffgenerator main.cpp)
target_compile_features(ffgenerator PRIVATE cxx_std_20)

target_link_libraries(ffgenerator PRIVATE jacc fmt::fmt spdlog::spdlog argparse)

 
