add_executable(logger_test Logger_test.cpp)
target_link_libraries(logger_test ssnet_base)

add_executable(timestamp_test Timestamp_test.cpp)
target_link_libraries(timestamp_test ssnet_base)