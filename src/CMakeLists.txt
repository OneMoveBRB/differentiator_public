add_library(app_core STATIC
    diff_tree.c
    io.c
    parser.c
    diff_math/differentiation.c
    diff_math/optimizations.c
)

target_include_directories(app_core PUBLIC
    $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>
)

target_link_libraries(app_core
    PUBLIC
        clibs::stack
        clibs::hash_table
        clibs::ring_queue
        clibs::dyn_arr
    PRIVATE
        project_warnings
        project_sanitizers
)

find_library(MATH_LIBRARY m)

if(MATH_LIBRARY)
    target_link_libraries(app_core PRIVATE ${MATH_LIBRARY})
endif()

#target_compile_definitions(app_core
#    PRIVATE
#        $<$<CONFIG:Debug>:DEBUG>
#)

add_library(app::core ALIAS app_core)

add_executable(run main.c)

target_link_libraries(run PRIVATE app::core)

