add_library(project_defines INTERFACE)

target_compile_definitions(project_defines INTERFACE
    $<$<CONFIG:Debug>:DEBUG>
)

target_compile_definitions(project_defines INTERFACE
    $<$<NOT:$<CONFIG:Debug>>:NDEBUG>
)

