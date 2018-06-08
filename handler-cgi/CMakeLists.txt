
include_directories(../headers/)

add_executable (handler-cgi handler-cgi.cpp)

target_link_libraries(handler-cgi LINK_PUBLIC utils)


