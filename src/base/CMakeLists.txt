file(GLOB base_sources "*.cpp")

add_library(ssnet_base ${base_sources})
target_link_libraries(ssnet_base pthread rt)

install(TARGETS ssnet_base DESTINATION lib)

file(GLOB HEADERS "*.h")
install(FILES ${HEADERS} DESTINATION include/src/base)

if (NOT CMAKE_BUILD_NO_EXAMPLES)
    add_subdirectory(tests)
endif ()