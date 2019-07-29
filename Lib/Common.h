#ifndef _COMMON_H_
#define _COMMON_H_

#include <cstdint>
#include <ctime>
#include <thread>
#include <unistd.h>
#include <cassert>

#define S2N 1000000000
#define CACHELINE_SIZE (64)

inline void bindCore(uint16_t core) {

    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    CPU_SET(core, &cpuset);
    int rc = pthread_setaffinity_np(pthread_self(), sizeof(cpu_set_t), &cpuset);
    assert(rc == 0);
}


// fast rand function
static uint64_t x = 123456789, y = 362436069, z = 521288629;
uint64_t inline xorshf96() {
    uint64_t t;
    x ^= x << 16;
    x ^= x >> 5;
    x ^= x << 1;

    t = x;
    x = y;
    y = z;
    z = t ^ x ^ y;

    return z;
}

class ThroughPutCounter {
  private:
    uint64_t lastValue;
    uint64_t &curCounter;
    uint8_t interval; // record interval (second)

    void start() {
        new std::thread([&] {
            timespec start, end;
            while (true) {
                clock_gettime(CLOCK_REALTIME, &start);
                sleep(interval);
                clock_gettime(CLOCK_REALTIME, &end);

                double microseconds =
                    (end.tv_sec - start.tv_sec) * 1000000 +
                    (double)(end.tv_nsec - end.tv_nsec) / 1000;
                double rate = (curCounter - lastValue) * 1000000 / microseconds;
                lastValue = curCounter;

                fprintf(stderr, "rate: %.0f ops, (%.0f Mops), %fns/op\n", rate,
                        rate / 1000000, 1000000000ull / rate);
            }
        });
    }

  public:
    ThroughPutCounter(uint64_t *curCounter, uint8_t interval = 1)
        : lastValue(0), curCounter(*curCounter), interval(interval) {
        start();
    }
};

struct CacheLine {
    uint64_t v;

    operator uint64_t() const { return v; }
    CacheLine &operator =(uint64_t c) {
        v = c;
        return *this;
    }

} __attribute__((aligned(CACHELINE_SIZE)));

template <uint8_t N> class MultiCoreThroughPutCounter {
  private:
    CacheLine (&curCounter)[N];
    uint8_t interval; // record interval (second)

    void start() {
        new std::thread([&] {
            uint64_t lastValue = 0;
            uint64_t cur = 0;
            timespec start, end;
            while (true) {
                clock_gettime(CLOCK_REALTIME, &start);
                sleep(interval);
                clock_gettime(CLOCK_REALTIME, &end);

                cur = 0;
                for (int i = 0; i < N; ++i) {
                    cur += curCounter[i];
                }

                double microseconds =
                    (end.tv_sec - start.tv_sec) * 1000000 +
                    (double)(end.tv_nsec - end.tv_nsec) / 1000;
                double rate = (cur - lastValue) * 1000000 / microseconds;
                lastValue = cur;

                fprintf(stderr, "rate: %.0f ops, (%.0f Mops)\n", rate,
                        rate / 1000000);
            }
        });
    }

  public:
    MultiCoreThroughPutCounter(CacheLine (&curCounter)[N], uint8_t interval = 1)
        : curCounter(curCounter), interval(interval) {
        start();
    }
};

#endif
