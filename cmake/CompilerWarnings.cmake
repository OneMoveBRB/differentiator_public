add_library(project_warnings INTERFACE)

if(NOT ENABLE_WARNINGS)
    return()
endif()

set(WARNING_FLAGS
    -Wall
    -Wextra
    -Wpedantic
    -Wshadow
    -Wcast-align
    -Wunused
    -Wconversion
    -Wsign-conversion
    -Wnull-dereference
    -Wformat=2
    -Werror=return-type
    -Werror=uninitialized
)

if(WARNINGS_AS_ERRORS)
    list(APPEND WARNING_FLAGS -Werror)
endif()

target_compile_options(project_warnings INTERFACE
    $<$<COMPILE_LANG_AND_ID:C,GNU,Clang,AppleClang>:${WARNING_FLAGS}>
    $<$<COMPILE_LANG_AND_ID:CXX,GNU,Clang,AppleClang>:${WARNING_FLAGS}>
)
