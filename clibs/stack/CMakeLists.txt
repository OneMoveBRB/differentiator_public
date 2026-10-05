add_library(stack STATIC
    src/stack.c
    src/stack_dump.c
)

target_include_directories(stack PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

target_link_libraries(stack PRIVATE project_warnings)

add_library(clibs::stack ALIAS stack)
