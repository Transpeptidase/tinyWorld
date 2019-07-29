#ifndef __ABSTRACT_LOCK_H__
#define __ABSTRACT_LOCK_H__

#include <atomic>
#include <string>

class AbstractLock {
  public:
    virtual void lock() = 0;
    virtual void unlock() = 0;
    virtual std::string info() = 0;
};

#endif
