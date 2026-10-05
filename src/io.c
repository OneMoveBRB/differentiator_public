#include "differentiator/io.h"

#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <errno.h>

#include "dyn_arr/dyn_arr.h"

typedef enum {
    FILE_OK,
    FILE_UNDEF_ERROR,
    FILE_NO_PERMISSION_TO_ACCESS,
    FILE_NOT_EXIST,
    FILE_TOO_LONG_PATH,
    FILE_OVERFLOW
} FileStatus;

static FileStatus GetFileSize(const char* file_name, size_t* file_size);

char* ReadFile(const char* file_name) {
    char* file_buffer = NULL;

    size_t     file_size = 0;
    FileStatus file_status = GetFileSize(file_name, &file_size);
    assert( file_status == FILE_OK );

    FILE* fp = fopen(file_name, "r");
    if (fp == NULL) {
        return NULL;
    }

    if (!DYNARR_SET_CAP(file_buffer, file_size + 1)) {
        return NULL;
    }

    size_t read_cnt = fread(file_buffer, sizeof(char), file_size, fp);
    assert( file_size == read_cnt );

    file_buffer[file_size] = '\0';

    return file_buffer;
}

static FileStatus GetFileSize(const char* file_name, size_t* file_size) {
    assert( file_name != NULL );

    struct stat file_info = {0};
    if (stat(file_name, &file_info) != 0) {
        switch (errno) {
        case EACCES:
            return FILE_NO_PERMISSION_TO_ACCESS;

        case ENOENT:
            return FILE_NOT_EXIST;

        case ENAMETOOLONG:
            return FILE_TOO_LONG_PATH;

        case EOVERFLOW:
            return FILE_OVERFLOW;
        
        default:
            return FILE_UNDEF_ERROR;
        }
    }

    *file_size = (size_t)file_info.st_size;

    return FILE_OK;
}

