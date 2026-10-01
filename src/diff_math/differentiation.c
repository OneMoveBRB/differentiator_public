#include "../../include/diff_math/differentiation.h"

#include <stdio.h>
#include <assert.h>

#include "../../include/dsl.h"

static DiffNode* DifferentiateNode(DiffNode* node);

DiffTree* DifferentiateTree(DiffTree* tree) {
    assert( tree != NULL );

    DiffTree* new_tree = DiffTreeCtor();
    if (new_tree == NULL) {
        return NULL;
    }

    new_tree->root = DifferentiateNode(tree->root);
    DiffTreeUpdateSize(tree);

    return new_tree;
}

static DiffNode* DifferentiateNode(DiffNode* node) {
    if (node == NULL) return NULL;

    if (node->data.type == TOKEN_TYPE_VARIABLE) { return CONSTANT(1.0); }
    if (node->data.type == TOKEN_TYPE_CONSTANT) { return CONSTANT(0.0); }

    assert( node->data.type == TOKEN_TYPE_OPERATION );

    switch (node->data.operation) {
    case OPERATION_TYPE_ADD: 
        return ADD(dL, dR);

    case OPERATION_TYPE_SUB:
        return SUB(dL, dR);
    
    case OPERATION_TYPE_MUL:
        return ADD(MUL(dL, cR), MUL(cL, dR));
    
    case OPERATION_TYPE_DIV:
        return DIV(SUB(MUL(dL, cR), MUL(cL, dR)), POW(cR, CONSTANT(2.0)));

    case OPERATION_TYPE_POW:
        return MUL(POW(cL, cR), ADD(MUL(dR, LN(cL)), MUL(DIV(cR, cL), dL)));

    case OPERATION_TYPE_SQRT:
        return DIV(dR, MUL(CONSTANT(2.0), SQRT(cR)));

    case OPERATION_TYPE_LN:
        return DIV(dR, cR);

    case OPERATION_TYPE_LOG:
        // TODO
        assert(0);

    case OPERATION_TYPE_SIN:
        return MUL(COS(cR), dR);

    case OPERATION_TYPE_COS:
        return SUB(CONSTANT(0.0), MUL(SIN(cR), dR));

    case OPERATION_TYPE_TAN:
        return DIV(dR, POW(COS(cR), CONSTANT(2.0)));
    
    case OPERATION_TYPE_COT:
        return SUB(CONSTANT(0.0), DIV(dR, POW(SIN(cR), CONSTANT(2.0))));

    case OPERATION_TYPE_SINH:
        return MUL(COSH(cR), dR);

    case OPERATION_TYPE_COSH:
        return MUL(SINH(cR), dR);

    case OPERATION_TYPE_TANH:
        return DIV(dR, POW(COSH(cR), CONSTANT(2.0)));

    case OPERATION_TYPE_COTH:
        return SUB(CONSTANT(0.0), DIV(dR, POW(SINH(cR), CONSTANT(2.0))));

    case OPERATION_TYPE_ASIN:
        return DIV(dR, SQRT(SUB(CONSTANT(1.0), POW(cR, CONSTANT(2.0)))));

    case OPERATION_TYPE_ACOS:
        return SUB(CONSTANT(0.0), DIV(dR, SQRT(SUB(CONSTANT(1.0), POW(cR, CONSTANT(2.0))))));

    case OPERATION_TYPE_ATAN:
        return DIV(dR, ADD(CONSTANT(1.0), POW(cR, CONSTANT(2.0))));

    case OPERATION_TYPE_ACOT:
        return SUB(CONSTANT(0.0), DIV(dR, ADD(CONSTANT(1.0), POW(cR, CONSTANT(2.0)))));
    
    default:
        assert(0);
    }

    assert(0);

    return NULL;
}
