#include "hash_table/double_ht.h"

#include <stdalign.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <math.h>
#include <assert.h>

#include "hash_table/murmurhash3.h"

#define FREE(ptr)   \
    do {            \
        free(ptr);  \
        ptr = NULL; \
    } while (0);

typedef enum TombstoneStatus {
    TOMBSTONE_STATUS_EMPTY    = 0,
    TOMBSTONE_STATUS_DELETED  = 1,
    TOMBSTONE_STATUS_OCCUPIED = 2
} TombstoneStatus;

typedef struct Slot {
   TombstoneStatus tombstone;   // 4 -> 8 bytes (default)
   /* key and data are stored inline after tombstone */
} Slot;

typedef struct DoubleHT {
    Slot*  slots;
    size_t size;
    size_t capacity;

    size_t key_size;
    size_t data_size;
    size_t slot_size;
    
    size_t key_offset;
    size_t data_offset;
    
    size_t alignment;

    double max_load_factor;

    HashFunc hash_func;
    KeyCmp key_cmp;

} DoubleHT;

static        bool       DoubleHT_Rehash(DoubleHT* ht);

static inline size_t     AlignUp(size_t size, size_t alignment);
static inline bool       IsPowOf2(size_t number);
static inline size_t     NextPowOf2(size_t number);
static inline bool       IsEqual(double num1, double num2);

static inline HashResult DefaultHashes(const void* key, size_t key_size, size_t capacity);

static const unsigned int kInitialCapacity      = 8;
static const unsigned int kExpansionCoefficient = 2;
static const double       kDefaultMaxLoadFactor = 0.7;
static const double       kEps                  = 1e-7;
static const uint32_t     kHashSeed             = 0x9e3779b9;

DoubleHT* DoubleHT_Ctor(size_t capacity, size_t key_size, size_t data_size,
                                         size_t alignment, double max_load_factor,
                                         HashFunc hash_func, KeyCmp key_cmp) {
    if (key_size == 0 || data_size == 0) {
        return NULL;
    }

    capacity = capacity ? capacity : kInitialCapacity;

    if (IsPowOf2(capacity) == false) {
        capacity = NextPowOf2(capacity);
    }

    alignment = alignment ? alignment : alignof(void*);

    if (IsPowOf2(alignment) == false) {
        alignment = NextPowOf2(alignment);
    }

    DoubleHT* ht = (DoubleHT*)calloc(1, sizeof(DoubleHT));
    if (ht == NULL) {
        return NULL;
    }

    ht->key_size  = key_size;
    ht->data_size = data_size;
    ht->alignment = alignment;
    ht->max_load_factor = IsEqual(max_load_factor, 0.0) ? kDefaultMaxLoadFactor : max_load_factor;

    ht->key_offset  = AlignUp(sizeof(TombstoneStatus)    , alignment);
    ht->data_offset = AlignUp(ht->key_offset + key_size  , alignment);
    ht->slot_size   = AlignUp(ht->data_offset + data_size, alignment);

    ht->slots = (Slot*)calloc(capacity, ht->slot_size);
    if (ht->slots == NULL) {
        FREE(ht);
        return NULL;
    }

    ht->size = 0;
    ht->capacity = capacity;
    
    ht->hash_func = hash_func ? hash_func : DefaultHashes;
    ht->key_cmp = key_cmp ? key_cmp : memcmp;

    return ht;
}

DoubleHT* DoubleHT_Dtor(DoubleHT* ht) {
    if (ht == NULL) return NULL;

    FREE(ht->slots);
    FREE(ht);

    return NULL;
}

void* DoubleHT_Insert(DoubleHT* ht, const void* key, const void* data) {
    assert( ht   != NULL );
    assert( key  != NULL );
    assert( data != NULL );

    if ((double)ht->size / (double)ht->capacity >= ht->max_load_factor) {
        if (DoubleHT_Rehash(ht) == false) {
            return NULL;
        }
    }

    size_t capacity = ht->capacity;
    HashResult hashes = DefaultHashes(key, ht->key_size, capacity);
    size_t hash1 = hashes.h1;
    size_t hash2 = hashes.h2;
    
    for (size_t i = 0; i < capacity; i++) {
        size_t pos = (hash1 + i * hash2) & (capacity - 1);
        Slot* slot = (Slot*)((char*)ht->slots + pos * ht->slot_size);

        switch (slot->tombstone) {
        case TOMBSTONE_STATUS_EMPTY:
        case TOMBSTONE_STATUS_DELETED: {
            void* slot_key  = (char*)slot + ht->key_offset;
            void* slot_data = (char*)slot + ht->data_offset;

            memcpy(slot_key, key, ht->key_size);
            memcpy(slot_data, data, ht->data_size);

            slot->tombstone = TOMBSTONE_STATUS_OCCUPIED;
            ++ht->size;

            return slot_data;
        }

        case TOMBSTONE_STATUS_OCCUPIED: {
            void* slot_key  = (char*)slot + ht->key_offset;
            void* slot_data = (char*)slot + ht->data_offset;

            if (ht->key_cmp(key, slot_key, ht->key_size) == 0) {
                memcpy(slot_data, data, ht->data_size);

                return slot_data;
            }

            break;
        }

        default:
            assert(0);
        }
    }

    assert(0);
    return NULL;
}

void* DoubleHT_Search(DoubleHT* ht, const void* key) {
    assert(ht != NULL);

    size_t capacity = ht->capacity;
    HashResult hashes = DefaultHashes(key, ht->key_size, capacity);
    size_t hash1 = hashes.h1;
    size_t hash2 = hashes.h2;

    for (size_t i = 0; i < capacity; i++) {
        size_t pos = (hash1 + i * hash2) & (capacity - 1);
        Slot* slot = (Slot*)((char*)ht->slots + pos * ht->slot_size);
        
        switch (slot->tombstone) {
        case TOMBSTONE_STATUS_EMPTY: {
            return NULL;
        }

        case TOMBSTONE_STATUS_DELETED: {
            break;
        }

        case TOMBSTONE_STATUS_OCCUPIED: {
            void* slot_key  = (char*)slot + ht->key_offset;
            void* slot_data = (char*)slot + ht->data_offset;

            if (ht->key_cmp(key, slot_key, ht->key_size) == 0) {
                return slot_data;
            }

            break;
        }

        default:
            assert(0);
        }
    }

    return NULL;
}

void* DoubleHT_Delete(DoubleHT* ht, const void* key) {
    assert(ht != NULL);

    size_t capacity = ht->capacity;
    HashResult hashes = DefaultHashes(key, ht->key_size, capacity);
    size_t hash1 = hashes.h1;
    size_t hash2 = hashes.h2;

    for (size_t i = 0; i < capacity; i++) {
        size_t pos = (hash1 + i * hash2) & (capacity - 1);
        Slot* slot = (Slot*)((char*)ht->slots + pos * ht->slot_size);
        
        switch (slot->tombstone) {
        case TOMBSTONE_STATUS_EMPTY: {
            return NULL;
        }

        case TOMBSTONE_STATUS_DELETED: {
            break;
        }

        case TOMBSTONE_STATUS_OCCUPIED: {
            void* slot_key  = (char*)slot + ht->key_offset;
            void* slot_data = (char*)slot + ht->data_offset;

            if (ht->key_cmp(key, slot_key, ht->key_size) == 0) {
                slot->tombstone = TOMBSTONE_STATUS_DELETED;
                --ht->size;
                return slot_data;
            }

            break;
        }

        default:
            assert(0);
        }
    }

    return NULL;
}

// static -------------------------------------------------------------------------

static bool DoubleHT_Rehash(DoubleHT* ht) {
    assert( ht != NULL );

    Slot* old_slots = ht->slots;
    size_t old_size = ht->size;
    size_t old_capacity = ht->capacity;

    size_t new_capacity = ht->capacity * kExpansionCoefficient;
    Slot* new_slots = (Slot*)calloc(new_capacity, ht->slot_size);
    if (new_slots == NULL) {
        return false;
    }
    
    ht->slots = new_slots;
    ht->size = 0;
    ht->capacity = new_capacity;

    for (size_t i = 0; i < old_capacity; i++) {
        Slot* old_slot = (Slot*)((char*)old_slots + i * ht->slot_size);

        if (old_slot->tombstone == TOMBSTONE_STATUS_OCCUPIED) {
            void* key  = (char*)old_slot + ht->key_offset;
            void* data = (char*)old_slot + ht->data_offset;

            DoubleHT_Insert(ht, key, data);
        }
    }

    FREE(old_slots);

    assert( old_size == ht->size );

    return true;
}

static inline size_t AlignUp(size_t size, size_t alignment) {
    return (size + alignment - 1) & ~(alignment - 1);
}

static inline bool IsPowOf2(size_t number) {
    return (number > 0) && ((number & (number - 1)) == 0);
}

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

static inline bool IsEqual(double num1, double num2) {
    return fabs(num1 - num2) < kEps;
}

static inline HashResult DefaultHashes(const void* key, size_t key_size, size_t capacity) {
    HashResult hash_result = {0};
    size_t   capacity_mask = capacity - 1;

#ifdef DOUBLE_HT_64_BIT
    Hash128_64 hash = MurmurHash3_x64_128(key, key_size, kHashSeed);
    hash_result.h1 = hash.h1 & capacity_mask;
    hash_result.h2 = (hash.h2 | 1) & capacity_mask;
    
#else
    Hash128_32 hash = MurmurHash3_x86_128(key, key_size, kHashSeed);
    hash_result.h1 = (hash.h1 ^ hash.h2) & capacity_mask;
    hash_result.h2 = ((hash.h3 ^ hash.h4) | 1) & capacity_mask;

#endif /* DOUBLE_HT_64_BIT */

    return hash_result;
}
