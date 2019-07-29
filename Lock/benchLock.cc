
#include <iostream>
#include <thread>

#include <time.h>

#include "Lock.h"

#include "LockStat.h"

#include "../Lib/Common.h"
#include "../Lib/Asm.h"
#include "../Lib/CommandLine.h"

#define MAX_THREAD 128

LockStat *stat[MAX_THREAD];
volatile uint64_t counter;
uint64_t lockTime;
int runTime = 5;

void benchThread(AbstractLock *lock, uint8_t core) {
    bindCore(core);

    stat[core] = new LockStat;

    while (true) {
        // for (int i = 0; i < 1000000; ++i) {
        lock->lock();

        stat[core]->succ++;
        counter++;

        uint64_t start = asm_rdtsc();
        while (asm_rdtsc() - start < lockTime)
            ;

        lock->unlock();
    }
}

int main(int argc, char *argv[]) {

    CommandLine cl(argc, argv);

    cl.addCommand('n', "thread number", "2");
    cl.addCommand('t', "cycle in locks");
    cl.addCommand('l', "lock type: tas | ttas | backoff | clh | mcs");

    cl.parseCommand();

    int threadNum = cl['n'];
    lockTime = cl['t'];

    std::thread th[MAX_THREAD];

    AbstractLock *lock = nullptr;
    std::string lockType = cl['l'];

    if (lockType == "tas") {
        lock = new TASLock();
    } else if (lockType == "ttas") {
        lock = new TTASLock();
    } else if (lockType == "backoff") {
        lock = new BackoffLock();
    } else if (lockType == "clh") {
        lock = new CLHLock();
    } else if (lockType == "mcs") {
        lock = new MCSLock();
    }

    if (!lock) {
        assert(false);
    }

    for (int i = 0; i < threadNum; ++i) {
        th[i] = std::thread(benchThread, lock, i);
    }

    sleep(1);

    struct timespec start, end;

    clock_gettime(CLOCK_REALTIME, &start);
    sleep(runTime);
    clock_gettime(CLOCK_REALTIME, &end);

    LockStat sum;
    for (uint8_t i = 0; i < threadNum; ++i) {
        // th[i].join();
        std::cerr << *stat[i] << " ";
        sum += *stat[i];
        th[i].detach();
    }
    std::cerr << counter << std::endl;
    std::cerr << std::endl << ((end.tv_sec * S2N + end.tv_nsec) -
                               (start.tv_sec * S2N + start.tv_nsec)) *
                                  1.0 / S2N << "s " << sum.succ << std::endl;

    return 0;
}
