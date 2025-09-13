cmake_minimum_required(VERSION 3.15)

project(
  jacc 
  VERSION 0.1
  DESCRIPTION "Jokkes amazing compiler compiler"
  LANGUAGES CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS on)

if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)

  # ensure -std=c++xx instead of -std=g++xx
  set(cmake_cxx_extensions off)

endif()

include(FetchContent)

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

# library code is here
add_subdirectory(src)

# application code is here
add_subdirectory(app)

# test code is here
add_subdirectory(test)
