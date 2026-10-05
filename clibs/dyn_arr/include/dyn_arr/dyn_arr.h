#ifndef DYN_ARR_H
#define DYN_ARR_H

#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#ifndef DYN_ARR_REALLOC
#define DYN_ARR_REALLOC(p, s) realloc(p, s)
#endif // DYN_ARR_REALLOC

#ifndef DYN_ARR_FREE
#define DYN_ARR_FREE(p) free(p)
#endif // DYN_ARR_FREE

typedef struct {
    size_t size;
    size_t capacity;
} DynArrMetaData;

// Get ptr to DynArrMetaData
#define DYNARR_METADATA(p)     ((DynArrMetaData*)(p) - 1)

// ------------------------ General Information ------------------------

// Get signed size
#define DYNARR_SIZE(p)         ((p) ? (ptrdiff_t)DYNARR_METADATA(p)->size : 0)

// Get unsigned size
#define DYNARR_SIZE_U(p)       ((p) ? DYNARR_METADATA(p)->size : 0)

// Get capacity
#define DYNARR_CAP(p)          ((p) ? DYNARR_METADATA(p)->capacity : 0)

// Get last element
#define DYNARR_LAST(p)         ((p)[DYNARR_METADATA(p)->size - 1])

// ----------------------- Insertion and Deletion ----------------------

/**
 * @brief  Push
 * @return `1` if OK, else `0`
 */
#define DYNARR_PUSH(p, x)      (DYNARR_MAY_BE_GROW(p, 1)                                \
                                ? ((p)[DYNARR_METADATA(p)->size++] = (x), 1) : 0)

/**
 * @brief  Add `n` uninitialized elements
 * @return Ptr to the 1st of them if OK, else `NULL`
 */
#define DYNARR_ADD_N_PTR(p, n) (DYNARR_MAY_BE_GROW(p, n)                                \
                                ? ((n) ? (DYNARR_METADATA(p)->size += (n),              \
                                         &(p)[DYNARR_METADATA(p)->size - (n)])          \
                                       : (p))                                           \
                                : NULL)

/**
 * @brief  Add `n` uninitialized elements
 * @return Index to the 1st of them if OK, else `(size_t)-1`
 */
#define DYNARR_ADD_N_IDX(p, n) (DYNARR_MAY_BE_GROW(p, n)                                \
                                ? ((n) ? (DYNARR_METADATA(p)->size += (n),              \
                                          DYNARR_METADATA(p)->size - (n))               \
                                       :  DYNARR_SIZE(p))                               \
                                : (size_t)-1)

/**
 * @brief  Insert `x` at `i` index
 * @return `1` if OK, else `0`
 */
#define DYNARR_INS(p, i, x)    ((DYNARR_MOVE_FORWARD_N(p, i, 1) != NULL)                \
                                ? ((p)[i] = (x), 1) : 0)

/**
 * @brief  Move forward `n` elements: `[i, size - n - 1] -> [i + n, size - 1]`
 * @return New `&(p)[(i) + (n)]` after movement if OK, else `NULL`
 */
#define DYNARR_MOVE_FORWARD_N(p, i, n) ((DYNARR_ADD_N_IDX(p, n) == (size_t)-1)          \
                                ? NULL : memmove(&(p)[(i) + (n)], &(p)[(i)],            \
                                sizeof(*(p)) * (DYNARR_METADATA(p)->size - (n) - (i))))

/**
 * @brief  Pop
 * @return Last element
 */
#define DYNARR_POP(p)          (--DYNARR_METADATA(p)->size, (p)[DYNARR_METADATA(p)->size])

/**
 * @brief  Delete at `i` index
 * @return New `&(p)[(i)]` after deletion
 */
#define DYNARR_DEL(p, i)       DYNARR_MOVE_BACK_N(p, i, 1)

/**
 * @brief  Move back `n` elements: `[i + n, size - 1] -> [i, size - n - 1]`
 * @return New `&(p)[(i)]` after movement
 */
#define DYNARR_MOVE_BACK_N(p, i, n) (DYNARR_METADATA(p)->size -= (n),                   \
                                memmove(&(p)[(i)], &(p)[(i) + (n)],                     \
                                sizeof(*(p)) * (DYNARR_METADATA(p)->size - (i))))

/**
 * @brief  Delete at i index swapping with the last one
 * @return Only rvalue of `(p)[(i)]` after swapping
 */
#define DYNARR_DEL_SWAP(p, i)  (--DYNARR_METADATA(p)->size,                             \
                                (p)[(i)] = (p)[DYNARR_METADATA(p)->size])

// ------------------------- Memory Management -------------------------

/**
 * @brief  Free
 */
#define DYNARR_FREE(p)         (((p) ? DYN_ARR_FREE(DYNARR_METADATA(p)) : (void)0),     \
                                (p) = NULL)

/**
 * @brief  Dtor: execute f for each element + Free array
 */
#define DYNARR_DTOR(p, f)                           \
    for (size_t i = 0; i < DYNARR_SIZE_U(p); i++) { \
        f(&(p)[i]);                                 \
    }                                               \
    DYNARR_FREE(p);

/**
 * @brief  Set capacity
 * @return `1` if OK, `0` else
 */
#define DYNARR_SET_CAP(p, n)   DYNARR_GROW(p, 0, n)

/**
 * @brief  Check and grow
 * @return `1` if OK, `0` else
 */
#define DYNARR_MAY_BE_GROW(p, n) ((!(p) || DYNARR_METADATA(p)->size + n                 \
                                           > DYNARR_METADATA(p)->capacity)              \
                                  ? DYNARR_GROW(p, n, 0) : 1)

/**
 * @brief  DynArrGrow wrapper
 * @return `1` if OK, `0` else
 */
#define DYNARR_GROW(p, s, c)   (DynArrGrowSafe((void**)&(p), sizeof(*(p)), (s), (c)))

static inline size_t NextPowOf2(size_t number) {
    --number;
    number |= number >> 1;
    number |= number >> 2;
    number |= number >> 4;
    number |= number >> 8;
    number |= number >> 16;
#if defined(__x86_64__) || defined(__LP64__) || defined(_WIN64)
    number |= number >> 32;
#endif
    ++number;
    return number;
}

static void* DynArrGrow(void* arr, size_t elem_size, size_t add_size, size_t new_cap) {
    DynArrMetaData* meta_data = arr ? DYNARR_METADATA(arr) : NULL;
    size_t cur_size = meta_data ? meta_data->size : 0;
    size_t cur_cap  = meta_data ? meta_data->capacity : 0;
    size_t need_cap = cur_size + add_size;

    if (new_cap == 0 && need_cap <= cur_cap) {
        return arr;
    }

    if (new_cap == 0 || new_cap < need_cap) {
        new_cap = (need_cap < 4) ? 4 : NextPowOf2(need_cap);
    }

    size_t total_size = sizeof(DynArrMetaData) + new_cap * elem_size;
    DynArrMetaData* new_meta_data = (DynArrMetaData*)DYN_ARR_REALLOC(meta_data, total_size);
    if (new_meta_data == NULL) {
        return NULL;
    }

    new_meta_data->size = cur_size;
    new_meta_data->capacity = new_cap;

    return (void*)(new_meta_data + 1);
}

static inline int DynArrGrowSafe(void** arr, size_t elem_size, size_t add_size, size_t new_cap) {
    void* new_arr = DynArrGrow(*arr, elem_size, add_size, new_cap);
    if (new_arr == NULL) {
        return 0;
    }

    *arr = new_arr;
    return 1;
}

#endif /* DYN_ARR_H */
