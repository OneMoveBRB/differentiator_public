#ifndef MURMURHASH3_H
#define MURMURHASH3_H

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

typedef struct {
    uint64_t h1;
    uint64_t h2;
} Hash128_64;

typedef struct {
    uint32_t h1;
    uint32_t h2;
    uint32_t h3;
    uint32_t h4;
} Hash128_32;

Hash128_32 MurmurHash3_x86_128(const void* key, const uint32_t len, uint32_t seed);
Hash128_64 MurmurHash3_x64_128(const void* key, const uint64_t len, uint32_t seed);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* MURMURHASH3_H */
