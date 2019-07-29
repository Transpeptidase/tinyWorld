#ifndef __ASM_H__
#define __ASM_H__ 

#include <cstdint>

#define compiler_barrier() asm volatile("" : : : "memory");

static inline void asm_movnti(volatile uint64_t *addr, uint64_t val) {
    __asm__ __volatile__("movnti %1, %0" : "=m"(*addr) : "r"(val));
}

static inline void asm_clflush(volatile uint64_t *addr) {
    __asm__ __volatile__("clflush %0" : : "m"(*addr));
}

static inline void asm_mfence(void) { __asm__ __volatile__("mfence"); }

static inline unsigned long long asm_rdtsc(void) {
    unsigned hi, lo;
    __asm__ __volatile__("rdtsc" : "=a"(lo), "=d"(hi));
    return ((unsigned long long)lo) | (((unsigned long long)hi) << 32);
}


#endif /* __ASM_H__ */
