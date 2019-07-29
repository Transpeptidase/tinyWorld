#include <iostream>
#include <thread>
#include <time.h>
#include <unistd.h>

#include "../Lib/Common.h"

using namespace std;

/*
 *  random cacheline access
 *
 */
#define SIZE (1024 * 1024 * 1024)

uint64_t counter;
uint8_t *pool;
uint32_t clLen = SIZE / 64 - 2;

void read_seq() {
    int index = 0;
    while (true) {
        uint8_t *start = pool + index;
        index = (index + 64) % SIZE;
        int k = start[46];
        counter++;
    }
}

void write_seq() {
    int index = 0;

    while (true) {
        uint8_t *start = pool + index;
        index = (index + 64) % SIZE;
        start[46] = 1;
        counter++;
    }
}

void read_ram() {
    while (true) {
        uint8_t *start = pool + 64 * (xorshf96() % clLen);
        int k = start[46];
        counter++;
    }
}

void write_ram() {
    while (true) {
        uint8_t *start = pool + 64 * (xorshf96() % clLen);
        start[46] = 4;
        counter++;
    }
}

int main(int argc, char **argv) {

    if (argc != 3) {
        fprintf(stderr, "Usage: ./a.out r|w seq|ram\n");
        return 0;
    }

    ThroughPutCounter(&counter, 2);

    pool = (uint8_t *)malloc(SIZE);  // 1G
    for (int i = 0; i < SIZE / 4; ++i) {
        *((uint32_t *)(pool) + i) = xorshf96();
    }

    if (argv[1][0] == 'r') {
        if (argv[2][0] == 's') {
            read_seq();
        } else {
            read_ram();
        }
    } else {
        if (argv[2][0] == 's') {
            write_seq();
        } else {
            write_ram();
        }
    }

    return 0;
}
