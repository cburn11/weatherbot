set(CMAKE_EXE_LINKER_FLAGS "-lboost_system -lcrypto -lssl -lcpprest")

add_executable (cache-forecast cache-forecast.cpp)

target_link_libraries(cache-forecast LINK_PUBLIC rest)


