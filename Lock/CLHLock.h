#ifndef __CLHLOCK_H__
#define __CLHLOCK_H__

#include "AbstractLock.h"

#include <atomic>
#include <thread>

class CLHLock : public AbstractLock {
   private:
    using QNode = bool;
    std::atomic<QNode*> tail;
    static thread_local QNode* myPred;
    static thread_local QNode* myNode;

   public:
    CLHLock() { tail.store(new QNode(false)); }

    void lock() override {
        *myNode = true;

        // getAndSet
        QNode* t;
        while (true) {
            t = tail.load();
            if (tail.compare_exchange_strong(t, myNode)) {
                break;
            }
        }

        myPred = t;
        while (*t) {
        }
    }

    void unlock() override {
        *myNode = false;
        myNode = myPred;
    }

    std::string info() override {
        return std::string("clh lock");
    }
};

#endif /* __CLHLOCK_H__ */
