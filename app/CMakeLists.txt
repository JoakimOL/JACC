add_executable(ffgenerator main.cpp)
target_compile_features(ffgenerator PRIVATE cxx_std_20)

target_link_libraries(ffgenerator PRIVATE jacc spdlog::spdlog argparse)

 
