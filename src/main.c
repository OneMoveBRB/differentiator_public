#include <stdio.h>
#include <assert.h>

#include "differentiator/io.h"
#include "dyn_arr/dyn_arr.h"
#include "differentiator/parser.h"
#include "differentiator/diff_tree.h"
#include "differentiator/diff_math/differentiation.h"
#include "differentiator/diff_math/optimizations.h"

static const char* gInputFile = NULL;

int main(int argc, char* argv[]) {
    if (argc == 2) {
        gInputFile = argv[1];
    }

    char* input_file_buffer = ReadFile(gInputFile);

    DiffTree* tree = ParseFileBuffer(input_file_buffer);

    DYNARR_FREE(input_file_buffer);

    DiffTreeDump(tree, NULL);

    printf("Tree size: %zu\n", tree->size);

    DiffTree* diff_tree = DifferentiateTree(tree);

    DiffTreeDump(diff_tree, "debug/diff_tree_2.txt");

    OptimizeTree(diff_tree);

    DiffTreeDump(diff_tree, "debug/diff_tree_3.txt");

    tree = DiffTreeDtor(tree);
    diff_tree = DiffTreeDtor(diff_tree);

    return 0;
}
