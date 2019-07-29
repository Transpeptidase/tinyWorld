#ifndef _HASH_TABLE_H_
#define _HASH_TABLE_H_

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <immintrin.h>
#include <stdio.h>
#include <city.h>

#define HASH_SIMD

#define MALLOC_NEW_KV(newPtr, t, index, k, klen, v, vlen)                      \
    uint8_t *newPtr = (uint8_t *)malloc(klen + vlen + sizeof(KeyLenType) +     \
                                        sizeof(ValueLenType));                 \
    t->ptr[index] = newPtr;                                                    \
                                                                               \
    *(KeyLenType *)newPtr = klen;                                              \
    newPtr += sizeof(KeyLenType);                                              \
    memcpy(newPtr, k, klen);                                                   \
    newPtr += klen;                                                            \
    *(ValueLenType *)newPtr = vlen;                                            \
    newPtr += sizeof(ValueLenType);                                            \
    memcpy(newPtr, v, vlen);

#define FREE_KV(p, index, t, pre, bIndex)                                      \
    free(p);                                                                   \
    t->bitmap &= ~(0x1 << index);                                              \
    if (t->bitmap == 0x0) {                                                    \
        if (pre) {                                                             \
            pre->next = t->next;                                               \
        } else {                                                               \
            table[bIndex] = t->next;                                           \
        }                                                                      \
        free(t);                                                               \
    } else {                                                                   \
        t->ptr[index] = NULL;                                                  \
        t->tags[index] = 0;                                                    \
    }

typedef uint8_t KeyLenType;
typedef uint16_t ValueLenType;

struct Bucket {
    uint16_t tags[8];
    uint8_t *ptr[8];

    uint8_t bitmap;
    Bucket *next;
} __attribute__((packed));

template <uint32_t N> class HashTable {
  private:
    Bucket *table[N];


  public:
    HashTable() { memset(table, 0, sizeof(Bucket *) * N); }
    ~HashTable() {}
    uint8_t *get(uint64_t tag, const uint8_t *k, KeyLenType klen,
                 ValueLenType *vlen);

    bool put(uint64_t tag, const uint8_t *k, KeyLenType klen, const uint8_t *v,
             ValueLenType vlen);

    uint8_t *get(const uint8_t *k, KeyLenType klen, ValueLenType *vlen);

    bool put(const uint8_t *k, KeyLenType klen, const uint8_t *v,
             ValueLenType vlen);

    bool del(uint64_t tag, uint8_t *k, KeyLenType klen);
    bool del(uint8_t *k, KeyLenType klen);
};

template <uint32_t N>
uint8_t *HashTable<N>::get(uint64_t tag, const uint8_t *k, KeyLenType klen,
                           ValueLenType *vlen) {
    Bucket *t = table[(tag >> 16) % N];

    uint16_t tag16 = tag & 0xffff;
    if (tag16 == 0)
        tag16++;

#ifdef HASH_SIMD
    const __m128i target = _mm_set1_epi16(tag16);
#endif

    while (t) {
#ifdef HASH_SIMD
        __m128i ts = _mm_loadu_si128((__m128i *)t->tags);

        uint16_t mask = _mm_movemask_epi8(_mm_cmpeq_epi16(ts, target));

        while (mask != 0x0) {

            uint8_t index = __builtin_ctz(mask) >> 1;
            uint8_t *p = t->ptr[index];

            KeyLenType _kl = *(KeyLenType *)p;
            if (_kl == klen &&
                *(uint64_t *)(p + sizeof(KeyLenType)) == // KEY IS 8 Byte
                    *(uint64_t *)k) {
                p += sizeof(KeyLenType) + klen;
                *vlen = *(ValueLenType *)p;
                return p + sizeof(ValueLenType);
            }
            mask &= (0xffff << ((index + 1) << 1));
        }
#else
        for (int i = 0; i < 8; ++i) {
            if (t->tags[i] == tag16) {
                uint8_t *p = t->ptr[i];

                KeyLenType _kl = *(KeyLenType *)p;
                if (_kl == klen &&
                    *(uint64_t *)(p + sizeof(KeyLenType)) == // KEY IS 8 Byte
                        *(uint64_t *)k) {
                    p += sizeof(KeyLenType) + klen;
                    *vlen = *(ValueLenType *)p;
                    return p + sizeof(ValueLenType);
                }
            }
        }
#endif

        t = t->next;
    }
    return NULL;
}

template <uint32_t N>
bool HashTable<N>::put(uint64_t tag, const uint8_t *k, KeyLenType klen,
                       const uint8_t *v, ValueLenType vlen) {

    uint32_t bIndex = (tag >> 16) % N;

    uint16_t tag16 = tag & 0xffff;
    if (tag16 == 0)
        tag16++;

#ifdef HASH_SIMD
    const __m128i target = _mm_set1_epi16(tag16);
#endif

    Bucket *emptyBucket = NULL;
    uint8_t emptyIndex = 0;
    bool findEmpty = false;

    Bucket *pre = NULL;
    Bucket *t = table[bIndex];
    while (t) {
#ifdef HASH_SIMD
        __m128i ts = _mm_loadu_si128((__m128i *)t->tags);
        uint16_t mask = _mm_movemask_epi8(_mm_cmpeq_epi16(ts, target));

        while (mask != 0x0) {
            uint8_t index = __builtin_ctz(mask) >> 1;
            uint8_t *p = t->ptr[index];

            KeyLenType _kl = *(KeyLenType *)p;
            if (_kl == klen &&
                *(uint64_t *)(p + sizeof(KeyLenType)) == // KEY IS 8 Byte
                    *(uint64_t *)k) {
                free(p);
                MALLOC_NEW_KV(newPtr, t, index, k, klen, v, vlen);
                return true;
            }
            mask &= (0xffff << ((index + 1) << 1));
        }
#else
        for (int i = 0; i < 8; ++i) {
            if (t->tags[i] == tag16) {
                uint8_t *p = t->ptr[i];

                KeyLenType _kl = *(KeyLenType *)p;
                if (_kl == klen &&
                    *(uint64_t *)(p + sizeof(KeyLenType)) == // KEY IS 8 Byte
                        *(uint64_t *)k) {
                    free(p);
                    MALLOC_NEW_KV(newPtr, t, i, k, klen, v, vlen);

                    return true;
                }
            }
        }
#endif

        if (!findEmpty && t->bitmap != 0xff) {
            findEmpty = true;
            emptyBucket = t;
            emptyIndex = __builtin_ctz(~t->bitmap);
        }

        pre = t;
        t = t->next;
    }

    if (!findEmpty) {

        Bucket *b = (Bucket *)malloc(sizeof(Bucket));

        *(uint16_t *)&b->bitmap = 0; // bitmap and next
        *(uint64_t *)b->tags = 0;
        *(uint64_t *)(b->tags + 4) = 0;

        if (pre) {
            pre->next = b;
        } else {
            table[bIndex] = b;
        }

        emptyBucket = b;
        emptyIndex = 0;
    }

    // assert(emptyBucket != NULL && emptyIndex >= 0 && emptyIndex < 8);

    MALLOC_NEW_KV(newPtr, emptyBucket, emptyIndex, k, klen, v, vlen);
    emptyBucket->tags[emptyIndex] = tag16;
    emptyBucket->bitmap |= (0x1 << emptyIndex);

    return true;
}

template <uint32_t N>
bool HashTable<N>::del(uint64_t tag, uint8_t *k, KeyLenType klen) {

    uint32_t bIndex = (tag >> 16) % N;

    uint16_t tag16 = tag & 0xffff;
    if (tag16 == 0)
        tag16++;

#ifdef HASH_SIMD
    const __m128i target = _mm_set1_epi16(tag16);
#endif

    Bucket *pre = NULL;
    Bucket *t = table[bIndex];
    while (t) {
#ifdef HASH_SIMD
        __m128i ts = _mm_loadu_si128((__m128i *)t->tags);
        uint16_t mask = _mm_movemask_epi8(_mm_cmpeq_epi16(ts, target));

        while (mask != 0x0) {
            uint8_t index = __builtin_ctz(mask) >> 1;
            uint8_t *p = t->ptr[index];

            KeyLenType _kl = *(KeyLenType *)p;
            if (_kl == klen &&
                *(uint64_t *)(p + sizeof(KeyLenType)) == // KEY IS 8 Byte
                    *(uint64_t *)k) {
                FREE_KV(p, index, t, pre, bIndex);
                return true;
            }
            mask &= (0xffff << ((index + 1) << 1));
        }
#else
        for (int i = 0; i < 8; ++i) {
            if (t->tags[i] == tag16) {
                uint8_t *p = t->ptr[i];

                KeyLenType _kl = *(KeyLenType *)p;
                if (_kl == klen &&
                    *(uint64_t *)(p + sizeof(KeyLenType)) == // KEY IS 8 Byte
                        *(uint64_t *)k) {
                    FREE_KV(p, i, t, pre, bIndex);
                    return true;
                }
            }
        }
#endif
        pre = t;
        t = t->next;
    }
    return false;
}

template <uint32_t N> bool HashTable<N>::del(uint8_t *k, KeyLenType klen) {
    return del(CityHash64((char *)k, klen) >> 24, k, klen);
}

template <uint32_t N>
uint8_t *HashTable<N>::get(const uint8_t *k, KeyLenType klen,
                           ValueLenType *vlen) {
    return get(CityHash64((char *)k, klen) >> 24, k, klen, vlen);
}

template <uint32_t N>
bool HashTable<N>::put(const uint8_t *k, KeyLenType klen, const uint8_t *v,
                       ValueLenType vlen) {
    return put(CityHash64((char *)k, klen) >> 24, k, klen, v, vlen);
}
#endif
