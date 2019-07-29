#include <iostream>
#include <vector>
#include <thread>
#include <algorithm>

// #include "BoundedQueue.h"
#include "LockFreeQueue.h"

#define EN_NUM 4
#define DE_NUM 8
#define ITEM_COUNT 100000

// BoundedQueue<int> q(100);
LockFreeQueue<int> q;

void enThread(int core) {

    (void)core;

    for (int i = 0; i < ITEM_COUNT / EN_NUM; ++i) {
        q.enq(i + 1);
    }
}

std::vector<int> output[DE_NUM];
void deThread(int core) {
    int res;
    for (int i = 0; i < ITEM_COUNT / DE_NUM; ++i) {

        while (!q.dep(res))
            ;
        output[core].push_back(res);
        ;
    }
}

int main() {

    std::thread te[EN_NUM];
    std::thread th[DE_NUM];

    for (int i = 0; i < EN_NUM; ++i) {
        te[i] = std::thread(enThread, i);
    }
    for (int i = 0; i < DE_NUM; ++i) {
        th[i] = std::thread(deThread, i);
    }

    for (int i = 0; i < EN_NUM; ++i) {
        te[i].join();
    }
    for (int i = 0; i < DE_NUM; ++i) {
        th[i].join();
    }

    std::vector<int> res;
    for (int i = 0; i < DE_NUM; ++i) {
        res.insert(res.end(), output[i].begin(), output[i].end());
    }

    std::sort(res.begin(), res.end());
    std::for_each(res.begin(), res.end(), [](int i) { std::cout << i << " "; });

    std::cout << std::endl;

    return 0;
}
