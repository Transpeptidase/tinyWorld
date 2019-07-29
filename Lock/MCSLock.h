#ifndef __MCSLOCK_H__
#define __MCSLOCK_H__

#include "AbstractLock.h"

#include <atomic>
#include <thread>

class MCSLock : public AbstractLock {
   private:
    struct QNode {
        bool locked;
        QNode* next;

        QNode() : locked(false), next(nullptr) {}
    };

    std::atomic<QNode*> tail;
    static thread_local QNode myNode;

   public:
    MCSLock() { tail.store(nullptr); }

    void lock() override {

        // getAndSet
        QNode* pred;
        while (true) {
            pred = tail.load();
            if (tail.compare_exchange_strong(pred, &myNode)) {
                break;
            }
        }

        if (pred) {
            myNode.locked = true;
            pred->next = &myNode;
            while (myNode.locked) {
            }
        }
    }

    void unlock() override {
        if (myNode.next == nullptr) {
            QNode* ref = &myNode;
            if (tail.compare_exchange_strong(ref, nullptr)) {
                return;
            }

            while (myNode.next == nullptr) {
            }
        }
        myNode.next->locked = false;
        myNode.next = nullptr;
    }

    std::string info() override { return std::string("mcs lock"); }
};

#endif /* __MCSLOCK_H__ */
