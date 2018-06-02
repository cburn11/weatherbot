set(CMAKE_EXE_LINKER_FLAGS "-lboost_system -lcrypto -lssl -lcpprest")

include_directories(../common/)

add_executable (weatherbotd weatherbot-daemon.cpp)

target_link_libraries(weatherbotd LINK_PUBLIC rest)
target_link_libraries(weatherbotd LINK_PUBLIC utils)
