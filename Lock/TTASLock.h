#ifndef __TTASLOCK_H__
#define __TTASLOCK_H__

#include "AbstractLock.h"

class TTASLock : public AbstractLock {
   private:
    std::atomic_bool l;
    const static bool UNLOCKED = false;
    const static bool LOCKED = true;

   public:
    TTASLock() { l.store(UNLOCKED); }

    void lock() override {
        while (true) {
            while (l.load() == LOCKED) {
                ;
            }

            bool f = UNLOCKED;
            if (l.compare_exchange_strong(f, LOCKED)) {
                break;
            }
        }
    }

    void unlock() override { l.store(UNLOCKED); }

    std::string info() override {
        return std::string("test and test_and_set lock");
    }
};

#endif /* __TASLOCK_H__ */
