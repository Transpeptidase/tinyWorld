#ifndef __BOUNDEDQUEUE_H__
#define __BOUNDEDQUEUE_H__

#include <atomic>
#include <condition_variable>
#include <queue>

template <class T>
struct Node {
    T item;
    Node *next;

    Node() {}
    Node(const T &x) : item(x), next(nullptr) {}
};

template <class T>
class BoundedQueue {
   private:
    std::mutex enqMut, depMut;
    std::condition_variable notEmpty, notFull;
    std::atomic_int size;

    Node<T> *head, *tail;

    const int capacity;

   public:
    BoundedQueue(int capacity) : size(0), capacity(capacity) {
        head = new Node<T>();
        tail = head;
    }

    void enq(const T &x) {

        bool wakeDequeuers = false;

        {
            std::unique_lock<std::mutex> enqLock(enqMut);
            notFull.wait(enqLock, [this] { return size.load() != capacity; });

            Node<T> *e = new Node<T>(x);
            tail->next = e;
            tail = e;

            if (size.fetch_add(1) == 0) {
                wakeDequeuers = true;
            }
        }

        if (wakeDequeuers) {
            std::unique_lock<std::mutex> depLock(depMut);
            notEmpty.notify_all();
        }
    };

    T dep() {
        T res;
        bool wakeEnqueuers = false;

        {
            std::unique_lock<std::mutex> depLock(depMut);
            notEmpty.wait(depLock, [this] { return size.load() != 0; });

            auto gc = head;
            res = head->next->item;
            head = head->next;
            delete gc;

            if (size.fetch_add(-1) == capacity) {
                wakeEnqueuers = true;
            }
        }

        if (wakeEnqueuers) {
            std::unique_lock<std::mutex> enqLock(enqMut);
            notFull.notify_all();
        }

        return res;
    }
};
#endif /* __BOUNDEDQUEUE_H__ */
