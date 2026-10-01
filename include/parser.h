#ifndef PARSER_H
#define PARSER_H

#include "diff_tree.h"
#include "tokens/token_def.h"

typedef enum {
    LEX_UNDEF   = 0,
    LEX_SUCCESS = 1,
    LEX_FAILURE = 2
} LexStatus;

DiffTree* ParseFileBuffer(char* file_buffer);

#endif /* PARSER_H */
