include_directories(../headers)
set(CMAKE_CXX_FLAGS "-fPIC")

FIND_PACKAGE ( Threads REQUIRED )

add_library (weatherbot_command SHARED weatherbot-command.cpp)

target_link_libraries(weatherbot_command LINK_PUBLIC rest_utils utils ${CMAKE_THREAD_LIBS_INIT})
