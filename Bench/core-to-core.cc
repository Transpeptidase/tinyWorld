#include <thread>

#include "../Lib/Common.h"

#define MAX_CORE 64

int CORE_NUM = 2;

struct alignas(CACHELINE_SIZE) Message {
    volatile uint64_t value;

    Message() : value(0) {}
};

Message send[MAX_CORE];
Message recv[MAX_CORE];

uint64_t counter = 0;

void receiver(int id) {

    bindCore(id);
    uint64_t cur = 1;

    while (true) {

        if (send[id].value == cur) {
            recv[id].value = cur;
            cur++;
        }
    }
}

void sender() {

    bindCore(0);
    uint64_t cur = 1;
    while (true) {

        for (int i = 1; i < CORE_NUM; ++i) {
            send[i].value = cur;
        }

        for (int i = 1; i < CORE_NUM; ++i) {
            while (recv[i].value != cur)
                ;
        }

        cur++;
        counter++;
    }
}

int main(int argc, char **argv) {

    if (argc != 2) {
        printf("./a.out thread number\n");
        exit(0);
    }
    CORE_NUM = std::atoi(argv[1]);
    printf("thread number %d\n", CORE_NUM);

    ThroughPutCounter(&counter, 2);

    new std::thread(sender);

    for (int i = 1; i < CORE_NUM; ++i) {
        new std::thread(receiver, i);
    }

    while (true)
        ;
}
