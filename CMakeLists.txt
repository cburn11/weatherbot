cmake_minimum_required (VERSION 3.7)

set(CMAKE_C_COMPILER "/usr/bin/clang")
set(CMAKE_CXX_COMPILER "/usr/bin/clang++")
set(CMAKE_CXX_COMPILER_ARG1 "-std=c++17")


project (WEATHERBOT)

add_subdirectory(rest)
add_subdirectory(utils)
add_subdirectory(handler-cgi)
add_subdirectory(weatherbot-standalone)
add_subdirectory(handler-daemon)
add_subdirectory(handler-test)
add_subdirectory(weatherbot-command)
add_subdirectory(need-command)

