file(GLOB net_sources "*.cpp")

add_library(ssnet_net_lib ${net_sources})
target_link_libraries(ssnet_net_lib ssnet_base pthread)

install(TARGETS ssnet_net_lib DESTINATION lib)

file(GLOB HEADERS "*.h")
install(FILES ${HEADERS} DESTINATION include/src/net)

if (NOT CMAKE_BUILD_NO_EXAMPLES)
    add_subdirectory(tests)
endif ()