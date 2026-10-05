add_library(hash_table STATIC
    src/double_ht.c
    src/murmurhash3.c
)

target_include_directories(hash_table PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

target_link_libraries(hash_table PRIVATE project_warnings)

add_library(clibs::hash_table ALIAS hash_table)
