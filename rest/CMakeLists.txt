
include_directories(../headers/)

find_package(cpprestsdk REQUIRED)

add_library (rest_utils SHARED http.cpp json_settings.cpp)

target_include_directories (rest_utils PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(rest_utils PRIVATE cpprestsdk::cpprest)


