#include "../include/parser.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>

#include "../include/dsl.h"
#include "../clibs/dyn_arr.h"

#define SKIP_SPACES(file_buffer, offset)                \
    while (isspace(file_buffer[offset])) { ++offset; }

static Token*    LexicalAnalysis(char* file_buffer);
static LexStatus LexParseBrackets(const char* file_buffer, size_t* offset, Token** tokens);
static LexStatus LexParseOperations(const char* file_buffer, size_t* offset, Token** tokens);
static LexStatus LexParseVariable(const char* file_buffer, size_t* offset, Token** tokens);
static LexStatus LexParseConstant(char* file_buffer, size_t* offset, Token** tokens);
static LexStatus LexParseComma(const char* file_buffer, size_t* offset, Token** tokens);

static DiffTree* SyntacticAnalysis(Token* tokens);
static DiffNode* GetAddSub(Token* tokens, size_t* tokens_idx);
static DiffNode* GetMulDiv(Token* tokens, size_t* tokens_idx);
static DiffNode* GetPow(Token* tokens, size_t* tokens_idx);
static DiffNode* GetPrimary(Token* tokens, size_t* tokens_idx);
static DiffNode* GetFunc(Token* tokens, size_t* token_idx);
static DiffNode* GetVar(Token* tokens, size_t* token_idx);
static DiffNode* GetConst(Token* tokens, size_t* token_idx);
static inline size_t NextTokenIdx(size_t token_idx);
static inline void TokenDtor(Token* token);

DiffTree* ParseFileBuffer(char* file_buffer) {
    assert( file_buffer != NULL );

    Token* tokens = LexicalAnalysis(file_buffer);
    if (tokens == NULL) {
        fprintf(stderr, "tokens == NULL\n");
        return NULL;
    }

    for (size_t i = 0; i < DYNARR_SIZE_U(tokens); i++) {
        printf("%d ", tokens[i].type);
    }   printf("\n");

    DiffTree* tree = SyntacticAnalysis(tokens);
    if (tree == NULL) {
        fprintf(stderr, "tree == NULL\n");
        DYNARR_DTOR(tokens, TokenDtor);
        return NULL;
    }

    DYNARR_DTOR(tokens, TokenDtor);

    return tree;
}

// ----------------------------------- Lexer -----------------------------------

static Token* LexicalAnalysis(char* file_buffer) {
    assert( file_buffer != NULL );

    size_t offset = 0;
    Token* tokens = NULL;

    LexStatus status = LEX_UNDEF;
    while (offset < DYNARR_CAP(file_buffer) - 1) {
        SKIP_SPACES(file_buffer, offset);

        status = LexParseBrackets(file_buffer, &offset, &tokens);
        if (status == LEX_SUCCESS) continue;

        status = LexParseOperations(file_buffer, &offset, &tokens);
        if (status == LEX_SUCCESS) continue;

        status = LexParseVariable(file_buffer, &offset, &tokens);
        if (status == LEX_SUCCESS) continue;

        status = LexParseConstant(file_buffer, &offset, &tokens);
        if (status == LEX_SUCCESS) continue;

        status = LexParseComma(file_buffer, &offset, &tokens);
        if (status == LEX_SUCCESS) continue;
    }

    DYNARR_PUSH(tokens, ((Token){TOKEN_TYPE_NULL_TERMINATOR, {0}}));

    for (size_t i = 0; i < DYNARR_SIZE_U(tokens); i++) {
        printf("%d ", tokens[i].type);
    }

    return tokens;
}

static LexStatus LexParseBrackets(const char* file_buffer, size_t* offset, Token** tokens) {
    assert( file_buffer != NULL );

    Token  br_token     = {};
    size_t br_name_size = 0;

#define BRACKET(br_id, br_name, br_enum)                                                        \
    if (DYNARR_CAP(file_buffer) - 1 - *offset >= (br_name_size = strlen(br_name))               \
        && strncmp(&file_buffer[*offset], br_name, br_name_size) == 0) {                        \
        br_token = {TOKEN_TYPE_BRACKET, {.bracket = br_enum}};                                  \
        DYNARR_PUSH(*tokens, br_token);                                                         \
        *offset += br_name_size;                                                                \
        return LEX_SUCCESS;                                                                     \
    }

#include "../include/tokens/token_brackets.inc"

#undef BRACKET

    return LEX_FAILURE;
}

static LexStatus LexParseOperations(const char* file_buffer, size_t* offset, Token** tokens) {
    assert( file_buffer != NULL );

    Token  op_token     = {};
    size_t op_name_size = 0;

#define OPERATION(op_id, op_name, op_enum)                                                      \
    if (DYNARR_CAP(file_buffer) - 1 - *offset >= (op_name_size = strlen(op_name))               \
        && strncmp(&file_buffer[*offset], op_name, op_name_size) == 0) {                        \
        op_token = {TOKEN_TYPE_OPERATION, {.operation = op_enum}};                              \
        DYNARR_PUSH(*tokens, op_token);                                                         \
        *offset += op_name_size;                                                                \
        return LEX_SUCCESS;                                                                     \
    }

#include "../include/tokens/token_operations.inc"

#undef OPERATION

    return LEX_FAILURE;
}

static LexStatus LexParseVariable(const char* file_buffer, size_t* offset, Token** tokens) {
    assert( file_buffer != NULL );

    size_t offset_begin = *offset; 
    size_t offset_iter  = offset_begin;

    if (!isalpha(file_buffer[offset_iter]) && file_buffer[offset_iter] != '_') {
        return LEX_FAILURE;
    }

    ++offset_iter;

    while (isalnum(file_buffer[offset_iter]) || file_buffer[offset_iter] == '_') {
        ++offset_iter;
    }

    char* variable = strndup(file_buffer + offset_begin, offset_iter - offset_begin);
    assert( variable != NULL );

    DYNARR_PUSH(*tokens, ((Token){TOKEN_TYPE_VARIABLE, {.variable = variable}}));
    *offset = offset_iter;

    return LEX_SUCCESS;
}

static LexStatus LexParseConstant(char* file_buffer, size_t* offset, Token** tokens) {
    assert( file_buffer != NULL );

    size_t offset_begin = *offset; 
    size_t offset_iter  = offset_begin;

    if (!isdigit((unsigned char)file_buffer[offset_iter++])) {
        return LEX_FAILURE;
    }

    while (isdigit((unsigned char)file_buffer[offset_iter])) { ++offset_iter; }

    if (file_buffer[offset_iter] == '.') {
        ++offset_iter;
        while (isdigit((unsigned char)file_buffer[offset_iter])) { ++offset_iter; }
    }
    
    char* end_of_constant_str = NULL;
    char  end_char = file_buffer[offset_iter];
    file_buffer[offset_iter] = '\0';
    
    double constant = strtod((const char*)file_buffer + offset_begin, &end_of_constant_str);
    assert( (size_t)(end_of_constant_str - file_buffer) == offset_iter );
    file_buffer[offset_iter] = end_char;

    DYNARR_PUSH(*tokens, ((Token){TOKEN_TYPE_CONSTANT, {.constant = constant}}));
    *offset = offset_iter;

    return LEX_SUCCESS;
}

static LexStatus LexParseComma(const char* file_buffer, size_t* offset, Token** tokens) {
    assert( file_buffer != NULL );

    if (file_buffer[*offset] != ',') {
        return LEX_FAILURE;
    }

    DYNARR_PUSH(*tokens, ((Token){TOKEN_TYPE_COMMA, {0}}));
    ++(*offset);

    return LEX_SUCCESS;
}

// ----------------------------------- Syntax -----------------------------------

static DiffTree* SyntacticAnalysis(Token* tokens) {
    assert( tokens != NULL );

    DiffTree* tree = DiffTreeCtor();
    if (tree == NULL) {
        return NULL;
    }

    size_t tokens_idx = 0;

    tree->root = GetAddSub(tokens, &tokens_idx);
    assert( tree->root != NULL );

    assert( tokens[tokens_idx].type == TOKEN_TYPE_NULL_TERMINATOR );

//NOTE - SIMD + bitmasks
    size_t tree_size = 0;
    for (tokens_idx = 0; tokens_idx < DYNARR_SIZE_U(tokens); tokens_idx++) {
        if (   tokens[tokens_idx].type == TOKEN_TYPE_OPERATION
            || tokens[tokens_idx].type == TOKEN_TYPE_VARIABLE
            || tokens[tokens_idx].type == TOKEN_TYPE_CONSTANT) {
            ++tree_size;
        }
    }

    tree->size = tree_size;

    return tree;
}

static DiffNode* GetAddSub(Token* tokens, size_t* tokens_idx) {
    assert( tokens != NULL );

    size_t cur_token_idx = *tokens_idx;

    DiffNode* res_node = GetMulDiv(tokens, &cur_token_idx);
    assert( res_node != NULL );

    DiffNode* right_node = NULL;
    OperationType operation = OPERATION_TYPE_UNDEF;
    while (tokens[cur_token_idx].type                         == TOKEN_TYPE_OPERATION 
            && ((operation = tokens[cur_token_idx].operation) == OPERATION_TYPE_ADD
                ||                                  operation == OPERATION_TYPE_SUB)) {

        cur_token_idx = NextTokenIdx(cur_token_idx);
        right_node = GetMulDiv(tokens, &cur_token_idx);
        assert( right_node != NULL );

        res_node = BINARY_OPERATION(res_node, right_node, operation);
    }

    *tokens_idx = cur_token_idx;
    
    return res_node;
}

static DiffNode* GetMulDiv(Token* tokens, size_t* tokens_idx) {
    assert( tokens != NULL );

    size_t cur_token_idx = *tokens_idx;

    DiffNode* res_node = GetPow(tokens, &cur_token_idx);
    assert( res_node != NULL );

    DiffNode* right_node = NULL;
    OperationType operation = OPERATION_TYPE_UNDEF;
    while (tokens[cur_token_idx].type                         == TOKEN_TYPE_OPERATION 
            && ((operation = tokens[cur_token_idx].operation) == OPERATION_TYPE_MUL
                ||                                  operation == OPERATION_TYPE_DIV)) {

        cur_token_idx = NextTokenIdx(cur_token_idx);
        right_node = GetPow(tokens, &cur_token_idx);
        assert( right_node != NULL );

        res_node = BINARY_OPERATION(res_node, right_node, operation);
    }

    *tokens_idx = cur_token_idx;
    
    return res_node;
}

static DiffNode* GetPow(Token* tokens, size_t* tokens_idx) {
    assert( tokens != NULL );

    size_t cur_token_idx = *tokens_idx;

    DiffNode* res_node = GetPrimary(tokens, &cur_token_idx); // primary_node_base
    assert( res_node != NULL );

    DiffNode* power_node = NULL;
    while (    tokens[cur_token_idx].type      == TOKEN_TYPE_OPERATION 
            && tokens[cur_token_idx].operation == OPERATION_TYPE_POW) {
        cur_token_idx = NextTokenIdx(cur_token_idx);
        power_node = GetPrimary(tokens, &cur_token_idx);
        assert( power_node != NULL );

        res_node = POW(res_node, power_node);
    }

    *tokens_idx = cur_token_idx;
    
    return res_node;
}

static DiffNode* GetPrimary(Token* tokens, size_t* tokens_idx) {
    assert( tokens != NULL );    

    DiffNode* res_node = NULL;
    if ((res_node = GetConst(tokens, tokens_idx)) != NULL) {
        return res_node;
    }

    if ((res_node = GetFunc(tokens, tokens_idx)) != NULL) {
        return res_node;
    }

    if ((res_node = GetVar(tokens, tokens_idx)) != NULL) {
        return res_node;
    }

    size_t cur_token_idx = *tokens_idx;

    if (   tokens[cur_token_idx].type    == TOKEN_TYPE_BRACKET
        && tokens[cur_token_idx].bracket == BRACKET_TYPE_OPEN) {
        cur_token_idx = NextTokenIdx(cur_token_idx);
    } else { assert(0); }

    res_node = GetAddSub(tokens, &cur_token_idx);

    if (   tokens[cur_token_idx].type    == TOKEN_TYPE_BRACKET
        && tokens[cur_token_idx].bracket == BRACKET_TYPE_CLOSE) {
        cur_token_idx = NextTokenIdx(cur_token_idx);
    } else { assert(0); }

    *tokens_idx = cur_token_idx;

    return res_node;
}

static DiffNode* GetFunc(Token* tokens, size_t* token_idx) {
    assert( tokens != NULL );

    size_t cur_token_idx = *token_idx;

    if (tokens[cur_token_idx].type != TOKEN_TYPE_OPERATION) {
        return NULL;
    }

    OperationType operation = tokens[cur_token_idx].operation;

    if(   operation == OPERATION_TYPE_ADD 
       || operation == OPERATION_TYPE_SUB
       || operation == OPERATION_TYPE_MUL
       || operation == OPERATION_TYPE_DIV
       || operation == OPERATION_TYPE_POW) {
        return NULL;
    }

    cur_token_idx = NextTokenIdx(cur_token_idx);

    DiffNode* expr_node = NULL;
    if (   tokens[cur_token_idx].type    == TOKEN_TYPE_BRACKET 
        && tokens[cur_token_idx].bracket == BRACKET_TYPE_OPEN) {
        cur_token_idx = NextTokenIdx(cur_token_idx);
    } else { assert(0); }

    expr_node = GetAddSub(tokens, &cur_token_idx);

    if (   tokens[cur_token_idx].type    == TOKEN_TYPE_BRACKET 
        && tokens[cur_token_idx].bracket == BRACKET_TYPE_CLOSE) {
        cur_token_idx = NextTokenIdx(cur_token_idx);
    } else { assert(0); }

    *token_idx = cur_token_idx;

    return UNARY_OPERATION(expr_node, operation);
}

static DiffNode* GetVar(Token* tokens, size_t* token_idx) {
    assert( tokens != NULL );

    size_t cur_token_idx = *token_idx;

    if (tokens[cur_token_idx].type != TOKEN_TYPE_VARIABLE) {
        return NULL;
    }

    ++(*token_idx);
    return VARIABLE(tokens[cur_token_idx].variable);
}

static DiffNode* GetConst(Token* tokens, size_t* token_idx) {
    assert( tokens != NULL );

    size_t cur_token_idx = *token_idx;

    if (tokens[cur_token_idx].type != TOKEN_TYPE_CONSTANT) {
        return NULL;
    }

    ++(*token_idx);
    return CONSTANT(tokens[cur_token_idx].constant);
}

static inline size_t NextTokenIdx(size_t token_idx) {
    return token_idx + 1;
}

static inline void TokenDtor(Token* token) {
    assert( token != NULL );

    if (token->type == TOKEN_TYPE_VARIABLE) {
        free(token->variable);
    }
}
