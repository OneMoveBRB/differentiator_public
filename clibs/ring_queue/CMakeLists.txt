add_library(ring_queue STATIC
    src/ring_queue.c
)

target_include_directories(ring_queue PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

target_link_libraries(ring_queue PRIVATE project_warnings)

add_library(clibs::ring_queue ALIAS ring_queue)
