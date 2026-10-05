add_library(project_sanitizers INTERFACE)

if(NOT ENABLE_SANITIZERS)
    return()
endif()

target_compile_options(project_sanitizers INTERFACE
    $<$<CONFIG:Debug>:-fsanitize=address,undefined;-fno-omit-frame-pointer>
)

target_link_options(project_sanitizers INTERFACE
    $<$<CONFIG:Debug>:-fsanitize=address,undefined>
)
