#ifndef __BACKOFF_LOCK_H__
#define __BACKOFF_LOCK_H__

#include "AbstractLock.h"
#include "Backoff.h"

class BackoffLock : public AbstractLock {
   private:
    const static bool UNLOCKED = false;
    const static bool LOCKED = true;

    const static int MIN_DELAY = 2;
    const static int MAX_DELAY = 100;

    std::atomic_bool l;
    Backoff backoff;

   public:
    BackoffLock() : backoff(MIN_DELAY, MAX_DELAY) { l.store(UNLOCKED); }

    void lock() override {
        while (true) {
            while (l.load() == LOCKED) {
                ;
            }

            bool f = UNLOCKED;
            if (l.compare_exchange_strong(f, LOCKED)) {
                break;
            }
            backoff.backoff();
        }
    }

    void unlock() override { l.store(UNLOCKED); }

    std::string info() override {
        return std::string("backoff test and test_and_set lock");
    }
};

#endif /* __TASLOCK_H__ */
