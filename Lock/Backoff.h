#ifndef __BACKOFF_H__
#define __BACKOFF_H__

#include "../Lib/Asm.h"

#include <cstdlib>

#include <algorithm>

class Backoff {

   private:
    const int minDelay, maxDelay;
    int limit;
    unsigned int seed;

   public:
    Backoff(int minDelay, int maxDelay)
        : minDelay(minDelay), maxDelay(maxDelay) {
        limit = minDelay;
        seed = asm_rdtsc();
    }

    void backoff() {
        int delay = rand_r(&seed) % limit;
        limit = std::min(maxDelay, limit * 2);

        uint64_t start = asm_rdtsc();
        while (asm_rdtsc() - start < (uint64_t)delay) {
            ;
        }
    }
};

#endif /* __BACKOFF_H__ */
