
include_directories(../common)

add_executable (weatherbot weatherbot-cgi.cpp)

target_link_libraries(weatherbot LINK_PUBLIC utils)


