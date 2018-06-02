cmake_minimum_required (VERSION 3.7)

set(CMAKE_C_COMPILER "/usr/bin/clang-5.0")
set(CMAKE_CXX_COMPILER "/usr/bin/clang++-5.0")
set(CMAKE_CXX_COMPILER_ARG1 "-std=c++17")


project (WEATHERBOT)

add_subdirectory(rest)
add_subdirectory(utils)
add_subdirectory(weatherbot-cgi)
add_subdirectory(weatherbot-daemon)
add_subdirectory(cache-forecast)

