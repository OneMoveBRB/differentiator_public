cmake_minimum_required(VERSION 3.20)

project(differentiator
    LANGUAGES C CXX
)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

option(ENABLE_SANITIZERS  "Enable ASan+UBSan (Debug)"       OFF)
option(ENABLE_WARNINGS    "Enable strict compiler warnings" ON )
option(WARNINGS_AS_ERRORS "Treat warnings as errors"        OFF)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
include(CompilerWarnings)
include(Sanitizers)

add_compile_definitions($<$<CONFIG:Debug>:DEBUG>)

add_subdirectory(clibs)
add_subdirectory(src)
