add_executable(buffer_test Buffer_unittest.cpp)
target_link_libraries(buffer_test ssnet_net_lib)

add_executable(endpoint_test EndPoint_test.cpp)
target_link_libraries(endpoint_test ssnet_net_lib)

add_executable(chan_test Channel_test.cpp)
target_link_libraries(chan_test ssnet_net_lib)

add_executable(eventloop_test EventLoop_test.cpp)
target_link_libraries(eventloop_test ssnet_net_lib ssnet_base)

add_executable(tcpserver_test TcpServer_test.cpp)
target_link_libraries(tcpserver_test ssnet_net_lib ssnet_base)

add_executable(tcpclient_test TcpClient_test.cpp)
target_link_libraries(tcpclient_test ssnet_net_lib ssnet_base)

add_executable(eventloop_threadtest EventLoopThread_test.cpp)
target_link_libraries(eventloop_threadtest ssnet_net_lib ssnet_base)

add_executable(eventloopthreadpool_test EventLoopThreadPool_test.cpp)
target_link_libraries(eventloopthreadpool_test ssnet_net_lib ssnet_base)

add_executable(timer_test Timer_test.cpp)
target_link_libraries(timer_test ssnet_net_lib ssnet_base)