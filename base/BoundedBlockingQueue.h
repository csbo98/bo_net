#ifndef BO_NET_BASE_BOUNDEDBLOCKINGQUEUE_H
#define BO_NET_BASE_BOUNDEDBLOCKINGQUEUE_H

#include "Mutex.h"
#include "Condition.h"

#include <boost/circular_buffer.hpp>
#include <assert.h>

namespace bo_net 
{

template<typename T>
class BoundedBlockingQueue : boost::noncopyable
{

    public:
        explicit BoundedBlockingQueue(int maxSize) 
            : mutex_(),
              notEmpty_(mutex_),
              notFull_(mutex_),
              queue_(maxSize)
        {
        }

    void put(const T &x) {
        MutexLockGuard lock(mutex_);
        while(queue_.full()) {
            notFull_.wait();
        }
        assert(!queue_.full());

        queue_.push_back(x);
        notEmpty_.notifyOne();
    }

    void put(T &&x) {
        MutexLockGuard lock(mutex_);
        while(queue_.full()) {
            notFull_.wait();
        }
        assert(!queue_.full());

        queue_.push_back(std::move(x));
        notEmpty_.notifyOne();

    }

    T take() {
        MutexLockGuard lock(mutex_);
        while(queue_.empty()) {
            notEmpty_.wait();
        }
        assert(!queue_.empty());

        T front(std::move(queue_.front()));
        queue_.pop_front();
        notFull_.notifyOne();

        return front;
    }

    bool empty() const {
        MutexLockGuard lock(mutex_);
        return queue_.empty();
    }

    bool full() const {
        MutexLockGuard lock(mutex_);
        return queue_.full();
    }


    size_t size() const {
        MutexLockGuard lock(mutex_);
        return queue_.size();
    }

    size_t capacity() const {
        MutexLockGuard lock(mutex_);
        return queue_.capacity();
    }

    private:
        mutable MutexLock mutex_;
        // 使用两个条件变量分别处理两种不同的情况
        Condition         notEmpty_ GUARDED_BY(mutex_);
        Condition         notFull_ GUARDED_BY(mutex_);
        boost::circular_buffer<T> queue_ GUARDED_BY(mutex_); // circular_buffer是一个环形缓冲区，提供了类似于deque那样的队列相关的接口，适合用来设计有界阻塞队列

        
};
 
} // namespace bo_net


# endif  // BO_NET_BASE_BOUNDEDBLOCKINGQUEUE_H