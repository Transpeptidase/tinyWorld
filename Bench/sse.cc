#include <immintrin.h>
#include <stdio.h>
#include <stdint.h>


// simd cmp
int main() {
    uint16_t a[] = {0, 2, 3, 1, 5, 6, 7, 1};

    __m128i x = _mm_loadu_si128((__m128i*)a);
    // __m128i y = _mm_loadu_si128((__m128i*)b);
    __m128i y = _mm_set1_epi16(1);

    __m128i cmp = _mm_cmpeq_epi16(x, y);

    uint16_t mask = _mm_movemask_epi8(cmp);

    while (mask != 0) {

        printf("--- %x\n", mask);
        uint8_t offset = __builtin_ctz(mask) / 2;
        printf("%d\n", offset);

        mask &= (0xffff << (offset + 1) * 2);
    }


    return 0;
}
