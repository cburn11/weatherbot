include_directories(../headers)
set(CMAKE_CXX_FLAGS "-fPIC")

FIND_PACKAGE ( Threads REQUIRED )

add_library (need_command SHARED need-command.cpp)

target_link_libraries(need_command LINK_PUBLIC rest_utils ${CMAKE_THREAD_LIBS_INIT})
