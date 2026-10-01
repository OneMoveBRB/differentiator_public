#ifndef DOUBLE_HT_H
#define DOUBLE_HT_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <stdint.h>
#include <stddef.h>

#include "murmurhash3.h"

#if defined(__x86_64__) || defined(__LP64__) || defined(_WIN64)
#define DOUBLE_HT_64_BIT 1

typedef Hash128_64 HashResult;

#else /* defined(__x86_64__) || defined(__LP64__) || defined(_WIN64) */
#define DOUBLE_HT_32_BIT 1

typedef Hash128_32 HashResult;

#endif /* defined(__x86_64__) || defined(__LP64__) || defined(_WIN64) */

typedef HashResult (*HashFunc)(const void* key, size_t key_size, size_t capacity);
typedef int        (*KeyCmp)(const void* key1, const void* key2, size_t key_size);

typedef struct DoubleHT DoubleHT;

DoubleHT* DoubleHT_Ctor(size_t capacity, size_t key_size, size_t data_size,
                        size_t alignment, double max_load_factor,
                        HashFunc hash_func, KeyCmp key_cmp);
DoubleHT* DoubleHT_Dtor(DoubleHT* ht);
void*     DoubleHT_Insert(DoubleHT* ht, const void* key, const void* data);
void*     DoubleHT_Search(DoubleHT* ht, const void* key);
void*     DoubleHT_Delete(DoubleHT* ht, const void* key);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* DOUBLE_HT_H */
