
include_directories(../headers/)

add_executable (handler-test handler-test.cpp)

target_link_libraries(handler-test LINK_PUBLIC utils)


