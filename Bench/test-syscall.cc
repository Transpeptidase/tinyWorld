#include <thread>
#include <unistd.h>
#include <sys/types.h>
#include <sys/time.h>

#include "../Lib/Common.h"

int main() {

    uint64_t counter;
    ThroughPutCounter(&counter, 2);
    struct timeval s;

    while (true) {
        // getuid();
        // sched_yield();
        gettimeofday(&s, NULL); // vDSO

        counter++;
    }

    return 0;
}
