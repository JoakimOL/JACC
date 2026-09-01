cmake_minimum_required(VERSION 3.20)

project(
  jacc 
  VERSION 0.1
  DESCRIPTION "Jokkes amazing compiler compiler"
  LANGUAGES CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS on)

if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)

  # ensure -std=c++xx instead of -std=g++xx
  set(cmake_cxx_extensions off)

  set(JACC_IS_TOP_LEVEL ON)
else()
  set(JACC_IS_TOP_LEVEL OFF)
endif()

option(JACC_BUILD_APP "Build jacc's own app/ executable" ${JACC_IS_TOP_LEVEL})
option(JACC_BUILD_TESTS "Build jacc's own test/ suite" ${JACC_IS_TOP_LEVEL})

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

FetchContent_Declare(
  bearpig
  GIT_REPOSITORY https://github.com/joakimol/bearpig
  GIT_TAG        1a208ae43551d8780d0df40d7bc1a0fdf4c36f89) 
FetchContent_MakeAvailable(bearpig)

# library code is here
add_subdirectory(src)

# application code is here
if(JACC_BUILD_APP)
  add_subdirectory(app)
endif()

# test code is here
if(JACC_BUILD_TESTS)
  add_subdirectory(test)
endif()
