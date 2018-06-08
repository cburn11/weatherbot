include_directories(../headers)

add_library (weatherbot_command SHARED weatherbot-command.cpp ../handler-daemon/Connections.cpp)

target_link_libraries(weatherbot_command LINK_PUBLIC rest_utils utils)
