#ifndef __LOCKFREEQUEUE_H__
#define __LOCKFREEQUEUE_H__

#include <atomic>
#include <list>
#include <algorithm>

extern std::atomic_int globalEpoch;

#define EPOCH_COUNT 3
struct ThreadEpoch {
    std::list<void *> gc[EPOCH_COUNT];
    std::atomic_int localEpoch;

    void enter() { localEpoch.store(globalEpoch.load()); }

    void free(void *ptr) { gc[localEpoch % EPOCH_COUNT].push_back(ptr); }

    void exit() { localEpoch.store(globalEpoch.load()); }
};

#define MAX_THREAD 8
ThreadEpoch threadEpoch[MAX_THREAD];
void gcThread() {
    auto start = std::begin(threadEpoch);
    auto end = std::end(threadEpoch);

    while (true) {
        if (std::all_of(start, end, [](const ThreadEpoch &e) {
                return e.localEpoch == globalEpoch.load();
            })) {
            int gcEpoch = (globalEpoch + 2) % EPOCH_COUNT;
            globalEpoch = globalEpoch.fetch_add(1);

            for (int i = 0; i < MAX_THREAD; ++i) {
                auto l = threadEpoch[i].gc[gcEpoch];
                for (auto ptr : l) {
                    free(ptr);
                    ;
                }
                l.clear();
            }
        }
    }
}

template <class T>
struct Node {
    T item;
    std::atomic<Node<T> *> next;

    Node(const T &item) : item(item), next(nullptr) {}
    Node() {}
};

template <class T>
class LockFreeQueue {
   private:
    std::atomic<Node<T> *> head, tail;

   public:
    LockFreeQueue() {
        Node<T> *e = new Node<T>;
        head.store(e);
        tail.store(e);
    }

    void enq(const T &item) {
        Node<T> *node = new Node<T>(item);

        while (true) {
            Node<T> *last = tail.load();
            Node<T> *next = last->next.load();
            if (last != tail.load()) continue;

            if (next == nullptr) {
                if ((last->next).compare_exchange_strong(next, node)) {
                    tail.compare_exchange_strong(last, node);
                    return;
                }
            } else {
                tail.compare_exchange_strong(last, next);
            }
        }
    }

    bool dep(T &res) {
        while (true) {
            Node<T> *first = head.load();
            Node<T> *last = tail.load();
            Node<T> *next = first->next.load();

            if (first != head.load()) continue;

            if (first == last) {
                if (next == nullptr) {
                    return false;
                }
                tail.compare_exchange_strong(last, next);
            } else {
                res = next->item;
                if (head.compare_exchange_strong(first, next)) {
                    delete first;
                    return true;
                }
            }
        }
    }
};

#endif /* __LOCKFREEQUEUE_H__ */
