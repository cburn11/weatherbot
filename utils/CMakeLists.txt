include_directories(../headers)

add_library (utils SHARED utils.cpp daemon.cpp)

target_include_directories (utils PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

