add_library(dyn_arr INTERFACE)

target_include_directories(dyn_arr INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

add_library(clibs::dyn_arr ALIAS dyn_arr)
