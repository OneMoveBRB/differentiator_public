CXX = g++ -std=c++20
CXXFLAGS_RELEASE = -DNDEBUG -O2
CXXFLAGS_DEBUG   = -g -O0 -DDEBUG                   \
                   -Wall -Wextra                    \
                   -Wpedantic                       \
                   -Wshadow                         \
                   -Wcast-align                     \
                   -Wunused                         \
                   -Wconversion                     \
                   -Wsign-conversion                \
                   -Wnull-dereference               \
                   -Wformat=2                       \
                   -Werror=return-type              \
                   -Werror=uninitialized            \
                   -fsanitize=address,undefined

.PHONY: run

SRC_DIR   = src
CLIBS_DIR = clibs
BIN_DIR   = bin

CLIBS_SRC = $(CLIBS_DIR)/stack/src/stack.c           $(CLIBS_DIR)/stack/src/stack_dump.c       \
            $(CLIBS_DIR)/hash_table/src/double_ht.c  $(CLIBS_DIR)/hash_table/src/murmurhash3.c \
            $(CLIBS_DIR)/ring_queue/src/ring_queue.c

run: main.c $(SRC_DIR)/io.c $(SRC_DIR)/parser.c $(SRC_DIR)/diff_tree.c $(SRC_DIR)/diff_math/differentiation.c $(SRC_DIR)/diff_math/optimizations.c $(CLIBS_SRC)
	 @mkdir -p $(BIN_DIR)
	 $(CXX) $(CXXFLAGS_DEBUG) -o $(BIN_DIR)/$@ $^
