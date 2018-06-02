set(CMAKE_EXE_LINKER_FLAGS "-lboost_system -lcrypto -lssl -lcpprest")

add_library (rest rest.cpp)

target_include_directories (rest PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

