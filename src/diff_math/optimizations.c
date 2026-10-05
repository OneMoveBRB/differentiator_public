#include "differentiator/diff_math/optimizations.h"

#include <math.h>
#include <assert.h>

#include "differentiator/dsl.h"
#include "differentiator/utils.h"

#define IS_EQUAL(ptr, val) \
    (ptr->data.type == TOKEN_TYPE_CONSTANT && IsEqual(ptr->data.constant, val))

static DiffNode* DiffNodeOptimization(DiffNode* node);
static DiffNode* ConstOptimizeAdd(DiffNode* node);
static DiffNode* ConstOptimizeSub(DiffNode* node);
static DiffNode* ConstOptimizeMul(DiffNode* node);
static DiffNode* ConstOptimizeDiv(DiffNode* node);
static DiffNode* ConstOptimizePow(DiffNode* node);
static DiffNode* FoldConstants(DiffNode* node);

void OptimizeTree(DiffTree* tree) {
    assert( tree != NULL );

    size_t old_tree_size = 0;

    do {
        old_tree_size = tree->size;
        tree->root = DiffNodeOptimization(tree->root);
        DiffTreeUpdateSize(tree);
    } while (old_tree_size != tree->size);
}

static DiffNode* DiffNodeOptimization(DiffNode* node) {
    assert( node != NULL );

    if (node->data.type != TOKEN_TYPE_OPERATION) {
        return node;
    }

    TokenType left_type  = TOKEN_TYPE_UNDEFINED;
    TokenType right_type = TOKEN_TYPE_UNDEFINED;

    if (node->left != NULL) {
        node->left = DiffNodeOptimization(node->left);
        left_type = node->left->data.type;
    }

    if (node->right != NULL) {
        node->right = DiffNodeOptimization(node->right);
        right_type = node->right->data.type;
    }

    if (   (left_type == TOKEN_TYPE_UNDEFINED || left_type == TOKEN_TYPE_CONSTANT) 
        && right_type == TOKEN_TYPE_CONSTANT) {
        return FoldConstants(node);
    }

    switch (node->data.operation) {
    case OPERATION_TYPE_ADD:
        return ConstOptimizeAdd(node);

    case OPERATION_TYPE_SUB:
        return ConstOptimizeSub(node);
    
    case OPERATION_TYPE_MUL:
        return ConstOptimizeMul(node);

    case OPERATION_TYPE_DIV:
        return ConstOptimizeDiv(node);

    case OPERATION_TYPE_POW:
        return ConstOptimizePow(node);

    default:
        break;
    }

    return node;
}

#define ConstOtimizationHandler(func_name, expressions)     \
static DiffNode* ConstOptimize##func_name(DiffNode* node) { \
    assert( node != NULL );                                 \
                                                            \
    DiffNode* new_node = NULL;                              \
                                                            \
    expressions                                             \
                                                            \
    DiffTreeDeleteSubtree(node);                            \
                                                            \
    return new_node;                                        \
}

ConstOtimizationHandler(
    Add,
    if      (IS_EQUAL(node->left,  0.0)) new_node = cR;
    else if (IS_EQUAL(node->right, 0.0)) new_node = cL;
    else    return node;
)

ConstOtimizationHandler(
    Sub,
    if      (IS_EQUAL(node->right, 0.0)) new_node = cL;
    else    return node;
)

ConstOtimizationHandler(
    Mul,
    if      (IS_EQUAL(node->left,  0.0)) new_node = CONSTANT(0.0);
    else if (IS_EQUAL(node->right, 0.0)) new_node = CONSTANT(0.0);
    else if (IS_EQUAL(node->left,  1.0)) new_node = cR;
    else if (IS_EQUAL(node->right, 1.0)) new_node = cL;
    else    return node;
)

ConstOtimizationHandler(
    Div,
    if      (IS_EQUAL(node->left,  0.0)) new_node = CONSTANT(0.0);
    else if (IS_EQUAL(node->right, 1.0)) new_node = cL;
    else    return node;
)

ConstOtimizationHandler(
    Pow,
    if      (IS_EQUAL(node->right, 0.0)) new_node = CONSTANT(1.0);
    else if (IS_EQUAL(node->left,  1.0)) new_node = CONSTANT(1.0);
    else if (IS_EQUAL(node->right, 1.0)) new_node = cL;
    else    return node;
)

static DiffNode* FoldConstants(DiffNode* node) {
    assert( node != NULL );

    double left_const  = 0.0;
    double right_const = node->right->data.constant;

    if (node->left != NULL) {
        left_const = node->left->data.constant;
        node->left = DiffNodeDtor(node->left);
    }

    node->right = DiffNodeDtor(node->right);

    switch (node->data.operation) {
    case OPERATION_TYPE_ADD:
        return CONSTANT(left_const + right_const);
    
    case OPERATION_TYPE_SUB:
        return CONSTANT(left_const - right_const);

    case OPERATION_TYPE_MUL:
        return CONSTANT(left_const * right_const);

    case OPERATION_TYPE_DIV:
        return CONSTANT(left_const / right_const);

    case OPERATION_TYPE_POW:
        return CONSTANT(pow(left_const, right_const));

    case OPERATION_TYPE_SQRT:
        return CONSTANT(sqrt(right_const));

    case OPERATION_TYPE_LN  :
        return CONSTANT(log(right_const));

    case OPERATION_TYPE_LOG :
        assert(0); // TODO
        
    case OPERATION_TYPE_SIN :
        return CONSTANT(sin(right_const));

    case OPERATION_TYPE_COS :
        return CONSTANT(cos(right_const));

    case OPERATION_TYPE_TAN :
        return CONSTANT(tan(right_const));

    case OPERATION_TYPE_COT :
        return CONSTANT(1.0 / tan(right_const));
        
    case OPERATION_TYPE_SINH:
        return CONSTANT(sinh(right_const));

    case OPERATION_TYPE_COSH:
        return CONSTANT(cosh(right_const));

    case OPERATION_TYPE_TANH:
        return CONSTANT(tanh(right_const));

    case OPERATION_TYPE_COTH:
        return CONSTANT(1.0 / tanh(right_const));

    case OPERATION_TYPE_ASIN:
        return CONSTANT(asin(right_const));

    case OPERATION_TYPE_ACOS:
        return CONSTANT(acos(right_const));

    case OPERATION_TYPE_ATAN:
        return CONSTANT(atan(right_const));

    case OPERATION_TYPE_ACOT:
        return CONSTANT(tanh(1.0 / right_const));

    case OPERATION_TYPE_UNDEF:

    default:
        assert(0);
    }

    assert(0);

    return node;
}
