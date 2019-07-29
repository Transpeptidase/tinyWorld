#ifndef __TASLOCK_H__
#define __TASLOCK_H__

#include "AbstractLock.h"

class TASLock : public AbstractLock {
   private:
    std::atomic_bool l;
    const static bool UNLOCKED = false;
    const static bool LOCKED = true;

   public:
    TASLock() { l.store(UNLOCKED); }

    void lock() override {
        while (true) {
            bool f = UNLOCKED;
            if (l.compare_exchange_strong(f, LOCKED)) {
                break;
            }
        }
    }

    void unlock() override { l.store(UNLOCKED); }

    std::string info() override { return std::string("test_and_set lock"); }
};

#endif /* __TASLOCK_H__ */
