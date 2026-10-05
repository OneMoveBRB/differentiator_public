#ifndef DSL_H
#define DSL_H

#define dL DifferentiateNode(node->left)
#define dR DifferentiateNode(node->right)
#define cL DiffTreeCopySubtree(node->left)
#define cR DiffTreeCopySubtree(node->right)

#define CONSTANT(x) \
    DiffNodeCtor(NULL, NULL, NULL, TOKEN_TYPE_CONSTANT, x)

#define VARIABLE(x) \
    DiffNodeCtor(NULL, NULL, NULL, TOKEN_TYPE_VARIABLE, x)

#define UNARY_OPERATION(right, x) \
    DiffNodeCtor(NULL, right, NULL, TOKEN_TYPE_OPERATION, x)

#define BINARY_OPERATION(left, right, x) \
    DiffNodeCtor(left, right, NULL, TOKEN_TYPE_OPERATION, x)

#define ADD(left, right) \
    BINARY_OPERATION(left, right, OPERATION_TYPE_ADD)

#define SUB(left, right) \
    BINARY_OPERATION(left, right, OPERATION_TYPE_SUB)

#define MUL(left, right) \
    BINARY_OPERATION(left, right, OPERATION_TYPE_MUL)

#define DIV(left, right) \
    BINARY_OPERATION(left, right, OPERATION_TYPE_DIV)

#define POW(left, right) \
    BINARY_OPERATION(left, right, OPERATION_TYPE_POW)

#define SQRT(right) \
    UNARY_OPERATION(right, OPERATION_TYPE_SQRT)

#define LN(right) \
	UNARY_OPERATION(right, OPERATION_TYPE_LN)

#define SIN(right) \
	UNARY_OPERATION(right, OPERATION_TYPE_SIN)

#define COS(right) \
	UNARY_OPERATION(right, OPERATION_TYPE_COS)

#define TAN(right) \
	UNARY_OPERATION(right, OPERATION_TYPE_TAN)

#define COT(right) \
	UNARY_OPERATION(right, OPERATION_TYPE_COT)

#define SINH(right) \
	UNARY_OPERATION(right, OPERATION_TYPE_SINH)

#define COSH(right) \
	UNARY_OPERATION(right, OPERATION_TYPE_COSH)

#define TANH(right) \
	UNARY_OPERATION(right, OPERATION_TYPE_TANH)

#define COTH(right) \
	UNARY_OPERATION(right, OPERATION_TYPE_COTH)

#define ASIN(right) \
	UNARY_OPERATION(right, OPERATION_TYPE_ASIN)

#define ACOS(right) \
	UNARY_OPERATION(right, OPERATION_TYPE_ACOS)

#define ATAN(right) \
	UNARY_OPERATION(right, OPERATION_TYPE_ATAN)

#define ACOT(right) \
	UNARY_OPERATION(right, OPERATION_TYPE_ACOT)

#endif /* DSL_H */
