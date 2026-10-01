#ifndef TOKEN_DEF_H
#define TOKEN_DEF_H

typedef enum {
    TOKEN_TYPE_UNDEFINED       = 0,
    TOKEN_TYPE_OPERATION       = 1,
    TOKEN_TYPE_VARIABLE        = 2,
    TOKEN_TYPE_CONSTANT        = 3,
    TOKEN_TYPE_BRACKET         = 4,
    TOKEN_TYPE_COMMA           = 5,
    TOKEN_TYPE_NULL_TERMINATOR = 6
} TokenType;

typedef enum {
#define OPERATION(op_id, op_name, op_enum)       \
    op_enum = op_id,

#include "token_operations.inc"

#undef OPERATION
} OperationType;

typedef enum {
#define BRACKET(br_id, br_name, br_enum)       \
    br_enum = br_id,

#include "token_brackets.inc"

#undef BRACKET
} BracketType;

typedef struct {
    TokenType type;
    union {
        char*         variable;
        double        constant;
        OperationType operation;
        BracketType   bracket;
    };
} Token;

#endif /* TOKEN_DEF_H */
