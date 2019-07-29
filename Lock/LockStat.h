#ifndef __LOCKSTAT_H__
#define __LOCKSTAT_H__

#include <cstdint>
#include <iostream>

struct LockStat {

    uint64_t succ;

    LockStat() : succ(0) {}

    LockStat operator+(const LockStat &l) const {
        LockStat res;
        res.succ = this->succ + l.succ;

        return res;
    }

    LockStat &operator+=(const LockStat &l) {
        this->succ += l.succ;

        return *this;
    }

    LockStat operator/(int n) {
        LockStat res;

        res.succ = this->succ / n;
        return res;
    }

    friend inline std::ostream &operator<<(std::ostream &os, LockStat &stat) {
        os << "succ cnt: " << stat.succ << std::endl;

        return os;
    }

};

#endif /* __LOCKSTAT_H__ */
