add_executable(pingpong_server PingPongServer.cpp)
target_link_libraries(pingpong_server ssnet_net_lib)

add_executable(pingpong_client PingPongClient.cpp)
target_link_libraries(pingpong_client ssnet_net_lib)
