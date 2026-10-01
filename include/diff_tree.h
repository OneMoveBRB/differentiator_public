#ifndef DIFF_TREE_H
#define DIFF_TREE_H

#include <stddef.h>

#include "tokens/token_def.h"

typedef struct DiffNode {
    Token  data;
    struct DiffNode* left;
    struct DiffNode* right;
    struct DiffNode* parent;
} DiffNode;

typedef struct {
    DiffNode* root;
    size_t size;
} DiffTree;

DiffTree* DiffTreeCtor();
DiffNode* DiffNodeCtor(DiffNode* left, DiffNode* right, DiffNode* parent, TokenType token_type, ...);
DiffNode* DiffNodeDtor(DiffNode* node);
DiffTree* DiffTreeDtor(DiffTree* tree);

DiffNode* DiffTreeCopySubtree(DiffNode* node);
DiffNode* DiffTreeDeleteSubtree(DiffNode* node);
void      DiffTreeUpdateSize(DiffTree* tree);

#ifdef DEBUG
int       DiffTreeDump(DiffTree* tree, const char* dump_file_name);
#endif /* DEBUG */

#endif /* DIFF_TREE_H */
