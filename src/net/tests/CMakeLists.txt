add_executable(_chantest Channel_test.cpp)
target_link_libraries(_chantest ssnet_net)

add_executable(eventloop_test EventLoop_test.cpp)
target_link_libraries(eventloop_test ssnet_net ssnet_base)

add_executable(tcpserver_test TcpServer_test.cpp)
target_link_libraries(tcpserver_test ssnet_net ssnet_base)

add_executable(tcpclient_test TcpClient_test.cpp)
target_link_libraries(tcpclient_test ssnet_net ssnet_base)

add_executable(eventloop_threadtest EventLoopThread_test.cpp)
target_link_libraries(eventloop_threadtest ssnet_net ssnet_base)

add_executable(eventloopthreadpool_test EventLoopThreadPool_test.cpp)
target_link_libraries(eventloopthreadpool_test ssnet_net ssnet_base)

add_executable(timer_test Timer_test.cpp)
target_link_libraries(timer_test ssnet_net ssnet_base)

if (BOOSTTEST_LIBRARY)
    add_executable(buffer_unittest Buffer_unittest.cpp)
    target_link_libraries(buffer_unittest ssnet_net boost_unit_test_framework)
    add_test(NAME buffer_unittest COMMAND buffer_unittest)

    add_executable(endpoint_unittest EndPoint_unittest.cpp)
    target_link_libraries(endpoint_unittest ssnet_net boost_unit_test_framework)
    add_test(NAME endpoint_unittest COMMAND endpoint_unittest)
endif ()