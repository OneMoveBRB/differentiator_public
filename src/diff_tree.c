#include "differentiator/diff_tree.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <assert.h>

#include "stack/stack.h"
#include "hash_table/double_ht.h"
#include "ring_queue/ring_queue.h"

#define FREE(ptr)           \
    do {                    \
        free(ptr);          \
        ptr = NULL;         \
    } while (0);

#define va_arg_enum(type) ((type)va_arg(args, int))

#ifdef DEBUG
static const char* kDiffTreeDumpFile = "debug/diff_tree.txt";

static void DiffNodeDump(FILE* fp, DiffNode* node);
#endif /* DEBUG */

DiffTree* DiffTreeCtor() {
    DiffTree* tree = (DiffTree*)calloc(1, sizeof(DiffTree));
    if (tree == NULL) {
        return NULL;
    }

    return tree;
}

DiffNode* DiffNodeCtor(DiffNode* left, DiffNode* right, DiffNode* parent, TokenType token_type, ...) {
    DiffNode* node = (DiffNode*)calloc(1, sizeof(DiffNode));
    if (node == NULL) {
        return NULL;
    }

    va_list args;
    va_start(args, token_type);

    switch (token_type) {
    case TOKEN_TYPE_OPERATION: {
        node->data.type = TOKEN_TYPE_OPERATION;
        node->data.operation = va_arg_enum(OperationType);
        break;
    }
    
    case TOKEN_TYPE_VARIABLE: {
        node->data.type = TOKEN_TYPE_VARIABLE;
        node->data.variable = strdup(va_arg(args, const char*));
        break;
    }

    case TOKEN_TYPE_CONSTANT: {
        node->data.type = TOKEN_TYPE_CONSTANT;
        node->data.constant = va_arg(args, double);
        break;
    }

    case TOKEN_TYPE_UNDEFINED:
    case TOKEN_TYPE_BRACKET:
    case TOKEN_TYPE_COMMA:

    default:
        assert(0);
    }

    node->left = left;
    node->right = right;
    node->parent = parent;

    return node;
}

DiffNode* DiffNodeDtor(DiffNode* node) {
    if (node == NULL) return NULL;

    if (node->data.type == TOKEN_TYPE_VARIABLE) {
        FREE(node->data.variable);
    }

    FREE(node);

    return NULL;
}

DiffTree* DiffTreeDtor(DiffTree* tree) {
    assert( tree != NULL );

    DiffTreeDeleteSubtree(tree->root);

    FREE(tree);

    return NULL;
}

static DiffNode* CopyDiffNode(DiffNode* node) {
    if (node == NULL) return NULL;

    DiffNode* copy_node = (DiffNode*)calloc(1, sizeof(DiffNode));
    if (copy_node == NULL) {
        return NULL;
    }

    if (node->data.type == TOKEN_TYPE_VARIABLE) {
        copy_node->data.type = TOKEN_TYPE_VARIABLE;
        copy_node->data.variable = strdup(node->data.variable);
    } else {
        copy_node->data = node->data;
    }
    
    return copy_node;
}

DiffNode* DiffTreeCopySubtree(DiffNode* node) {
    assert( node != NULL );

    DoubleHT* hash_table = DoubleHT_Ctor(0, sizeof(DiffNode*), sizeof(DiffNode*), sizeof(void*), 
                                         0.0, NULL, NULL);

    if (hash_table == NULL) {
        return NULL;
    }

    DiffNode* new_root  = CopyDiffNode(node);
    if (new_root == NULL) {
        hash_table = DoubleHT_Dtor(hash_table);
        return NULL;
    }

    DoubleHT_Insert(hash_table, &node, &new_root);

    RingQueue* bfs_queue = RingQueueCtor(0, sizeof(DiffNode*));
    if (bfs_queue == NULL) {
        new_root  = DiffNodeDtor(new_root);
        hash_table = DoubleHT_Dtor(hash_table);
        return NULL;
    }

    RingQueuePush(bfs_queue, &node);

    DiffNode* real_node  = NULL;
    DiffNode* copy_node  = NULL;
    DiffNode* copy_left  = NULL;
    DiffNode* copy_right = NULL;
    while (RingQueueEmpty(bfs_queue) == false) {
        real_node = *(DiffNode**)RingQueuePop(bfs_queue);
        copy_node = *(DiffNode**)DoubleHT_Delete(hash_table, &real_node);

        fprintf(stderr, "r: %p\t c: %p\n", (void*)real_node, (void*)copy_node);

        assert( real_node != NULL );
        assert( copy_node != NULL );

        if (real_node->left != NULL) {
            fprintf(stderr, "   -> l\n");
            copy_left = CopyDiffNode(real_node->left);
            copy_node->left = copy_left;
            DoubleHT_Insert(hash_table, &real_node->left, &copy_left);
            RingQueuePush(bfs_queue, &real_node->left);
        }

        if (real_node->right != NULL) {
            fprintf(stderr, "   -> r\n");
            copy_right = CopyDiffNode(real_node->right);
            copy_node->right = copy_right;
            DoubleHT_Insert(hash_table, &real_node->right, &copy_right);
            RingQueuePush(bfs_queue, &real_node->right);
        }
    }

    hash_table = DoubleHT_Dtor(hash_table);
    bfs_queue  = RingQueueDtor(bfs_queue);
    
    return new_root;
}

DiffNode* DiffTreeDeleteSubtree(DiffNode* node) {
    if (node == NULL) return NULL;

    Stack* stack = StackCtor(0, sizeof(DiffNode*), "debug/DiffTreeDeleteSubtreeStack");
    assert( stack != NULL );

    StackPush(stack, &node);

    DiffNode** node_ptr = NULL;
    while (!StackEmpty(stack)) {
        node_ptr = (DiffNode**)StackPop(stack);
        node     = *node_ptr;
        assert( node != NULL );

        if (node->left  != NULL) {
            StackPush(stack, &node->left);
        }

        if (node->right != NULL) {
            StackPush(stack, &node->right);
        }

        node = DiffNodeDtor(node);
        FREE(node_ptr);
    }

    stack = StackDtor(stack);

    return NULL;
}

void DiffTreeUpdateSize(DiffTree* tree) {
    assert( tree != NULL );

    if (tree->root == NULL) {
        tree->size = 0;
        return;
    }

    size_t new_size = 0;

    Stack* stack = StackCtor(0, sizeof(DiffNode*), "debug/DiffTreeUpdateSizeStack");
    assert( stack != NULL );

    StackPush(stack, &tree->root);

    DiffNode** node_ptr = NULL;
    DiffNode*  node     = NULL;
    while (!StackEmpty(stack)) {
        node_ptr = (DiffNode**)StackPop(stack);
        node     = *node_ptr;
        assert( node != NULL );

        if (node->left  != NULL) {
            StackPush(stack, &node->left);
        }

        if (node->right != NULL) {
            StackPush(stack, &node->right);
        }

        ++new_size;
        FREE(node_ptr);
    }

    stack = StackDtor(stack);

    tree->size = new_size;
}

#ifdef DEBUG
#define PTR_FMT(ptr) ((void*)(ptr))

int DiffTreeDump(DiffTree* tree, const char* dump_file_name) {
    assert( tree != NULL );

    Stack* stack = StackCtor(0, sizeof(DiffNode*), "debug/DiffTreeDumpStack");
    assert( stack != NULL );

    StackPush(stack, &tree->root);

    dump_file_name = dump_file_name ? dump_file_name : kDiffTreeDumpFile;

    FILE* fp = fopen(dump_file_name, "w");
    if (fp == NULL) {
        return 1;
    }

    fprintf(fp, "digraph Tree {\n\t");
    fprintf(fp, "rankdir=HR;\n\t");
    fprintf(fp, "node [shape=record, style=filled, fillcolor=lightblue];\n\t");
    fprintf(fp, "edge [fontsize=10,  color=black];\n\n\t");

    while (stack->meta.size > 0) {
        DiffNode** node_ptr = (DiffNode**)StackPop(stack);
        DiffNode*  node     = *node_ptr;
        assert( node != NULL );

        if (node->left  != NULL) {
            StackPush(stack, &node->left);
        }
        if (node->right != NULL) {
            StackPush(stack, &node->right);
        }

        DiffNodeDump(fp, node);

        FREE(node_ptr);
    }

    stack = StackDtor(stack);

    fprintf(fp, "\n}");

    fclose(fp);

    return 0;
}

static void DiffNodeDump(FILE* fp, DiffNode* node) {
    assert(  fp  != NULL );
    assert( node != NULL );

    switch (node->data.type) {
    case TOKEN_TYPE_OPERATION: {

#define OPERATION(op_id, op_name, op_enum)  \
    if (node->data.operation == op_enum) {                                          \
        fprintf(fp, "node%p [label=\"{{{<f0> %p "                                   \
                                    "| <f1> type = OPERATION "                      \
                                    "| <f2> data = %s}} "                           \
                                    "| {<f3> left: %p | <f4> right: %p}}\"];\n\t",  \
                    PTR_FMT(node), PTR_FMT(node), op_name,                          \
                    PTR_FMT(node->left), PTR_FMT(node->right));                     \
    }

#include "differentiator/tokens/token_operations.inc"

#undef OPERATION

        break;
    }

    case TOKEN_TYPE_VARIABLE: {
        fprintf(fp, "node%p [label=\"{{{<f0> %p "
                                    "| <f1> type = VARIABLE "
                                    "| <f2> data = %s}} "
                                    "| { <f3> left: %p | <f4> right: %p}}\"];\n\t", 
                PTR_FMT(node), PTR_FMT(node), node->data.variable, 
                PTR_FMT(node->left), PTR_FMT(node->right));
        break;
    }

    case TOKEN_TYPE_CONSTANT: {
        fprintf(fp, "node%p [label=\"{{{<f0> %p "
                                    "| <f1> type = NUMBER "
                                    "| <f2> data = %lg}} "
                                    "| { <f3> left: %p | <f4> right: %p}}\"];\n\t", 
                PTR_FMT(node), PTR_FMT(node), node->data.constant, 
                PTR_FMT(node->left), PTR_FMT(node->right));
        break;
    }

    case TOKEN_TYPE_UNDEFINED:
    case TOKEN_TYPE_BRACKET:
    case TOKEN_TYPE_COMMA:
    case TOKEN_TYPE_NULL_TERMINATOR:

    default:
        assert(0);
    }

    if (node->left != NULL) {
        if (node->left->parent == node) {
            fprintf(fp, "node%p:f3 -> node%p [color=red, dir=both, arrowhead=normal];\n\t", 
                        PTR_FMT(node), PTR_FMT(node->left));
        } else {
            fprintf(fp, "node%p:f3 -> node%p [color=red, arrowhead=normal];\n\t", 
                        PTR_FMT(node), PTR_FMT(node->left));
            fprintf(fp, "node%p -> node%p:f3 [color=red, arrowhead=normal];\n\t", 
                        PTR_FMT(node->left), PTR_FMT(node));
        }
    }

    if (node->right != NULL) {
        if (node->right->parent == node) {
            fprintf(fp, "node%p:f4 -> node%p [color=green, dir=both, arrowhead=normal];\n\t", 
                        PTR_FMT(node), PTR_FMT(node->right));
        } else {
            fprintf(fp, "node%p:f4 -> node%p [color=green, arrowhead=normal];\n\t", 
                        PTR_FMT(node), PTR_FMT(node->right));
            fprintf(fp, "node%p -> node%p:f4 [color=green, arrowhead=normal];\n\t", 
                        PTR_FMT(node->right), PTR_FMT(node));
        }
    }

}
#endif /* DEBUG */

