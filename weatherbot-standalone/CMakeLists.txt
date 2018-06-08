set(CMAKE_EXE_LINKER_FLAGS "-ldl")
#set(CMAKE_CXX_FLAGS "-g -O0")

include_directories(../headers)

add_executable (weatherbot-standalone standalone.cpp ../handler-daemon/Command.cpp)

target_link_libraries(weatherbot-standalone LINK_PUBLIC rest_utils)


