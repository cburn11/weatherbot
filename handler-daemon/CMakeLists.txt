set(CMAKE_EXE_LINKER_FLAGS "-ldl -Wl,--export-dynamic")
set(CMAKE_CXX_FLAGS "-fPIC")

include_directories(../headers/)

FIND_PACKAGE ( Threads REQUIRED )

add_executable (handlerd handler-daemon.cpp Connections.cpp Command.cpp)

target_link_libraries(handlerd LINK_PUBLIC utils rest_utils ${CMAKE_THREAD_LIBS_INIT})
