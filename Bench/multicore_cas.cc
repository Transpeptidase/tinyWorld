#include "../Lib/Common.h"
#include "../Lib/nvm.h"

#include <iostream>
#include <thread>


#define _GB_ (1024ull * 1024 * 1024)

#define MAX_CORE 64
CacheLine counter[MAX_CORE];

int main(int argc, char **argv) {
    auto dram = persistent::alloc_nvm(5 * _GB_ , "/dev/dax0.0");
    auto pmem = persistent::alloc_nvm(5 * _GB_, "/dev/dax2.0");

    int *x = (int *)((char *)(dram+ 64));
    *x = 0;

    if (argc != 3) {
        fprintf(stderr, "Usage: ./a.out core_num cas|faa|store|mfence|null\n");
        return 0;
    }

    int c = atoi(argv[1]);
    fprintf(stdout, "%d thread in different cores\n", c);

    for (int i = 0; i < MAX_CORE; ++i) {
        counter[i] = 0;
    }
    MultiCoreThroughPutCounter<MAX_CORE> tp(counter);

    switch (argv[2][0]) {
        case 'c':  // cas
        {
            for (int i = 0; i < c; ++i) {
                new std::thread(
                    [&](int coreID) {
                        bindCore(coreID);
                        while (true) {
                            __sync_val_compare_and_swap(x, 0, 1);
                            counter[coreID] = counter[coreID] + 1;
                        }
                    },
                    i);
            }
        } break;
        case 'f':  // faa
        {
            for (int i = 0; i < c; ++i) {
                new std::thread(
                    [&](int coreID) {
                        bindCore(coreID);
                        while (true) {
                            __sync_fetch_and_add(x, 1);
                            counter[coreID] = counter[coreID] + 1;
                        }
                    },
                    i);
            }
        } break;
        case 's':  // store
        {
            for (int i = 0; i < c; ++i) {
                new std::thread(
                    [&](int coreID) {
                        bindCore(coreID);
                        while (true) {
                            *x = coreID;
                            counter[coreID] = counter[coreID] + 1;
                        }
                    },
                    i);
            }
        } break;
        case 'n':  // null
        {
            for (int i = 0; i < c; ++i) {
                new std::thread(
                    [&](int coreID) {
                        bindCore(coreID);
                        int t = 0;
                        while (true) {
                            t++;
                            counter[coreID] = counter[coreID] + 1;
                        }
                    },
                    i);
            }
        } break;
        case 'm':  // mfence
        {
            for (int i = 0; i < c; ++i) {
                new std::thread(
                    [&](int coreID) {
                        bindCore(coreID);
                        while (true) {
                            asm volatile("mfence" ::: "memory");
                            counter[coreID] = counter[coreID] + 1;
                        }
                    },
                    i);
            }
        } break;
        default:
            fprintf(stderr, "Usage: ./a.out core_num cas|faa|store\n");
            return 0;
    }

    while (true) {
    }
    return 0;
}
