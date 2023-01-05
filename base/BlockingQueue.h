#ifndef BO_NET_BASE_BLOCKINGQUEUE_H
#define BO_NET_BASE_BLOCKINGQUEUE_H


#include "Condition.h"
#include "Mutex.h"

#include <deque>
#include <assert.h>


namespace bo_net
{

template<typename T>
class BlockingQueue : noncopyable
{
    public:
        using queue_type = std::deque<T>;

        BlockingQueue()
            : mutex_(),
              notEmpty_(mutex_),
              queue_()
        {
        }

        void put(const T &x) {
            MutexLockGuard lock(mutex_);
            queue_.push_back(x);
            notEmpty_.notifyOne();  // 已经使用GUARDED_BY注释声明了，必须在持有锁的时候notify，否则会在编译时出现警告
        }

        void put(T &&x) {
            MutexLockGuard lock(mutex_);
            queue_.push_back(std::move(x));  // 使用std::move继续保持右值引用
            notEmpty_.notifyOne();
        }

        T take() {
            MutexLockGuard lock(mutex_);
            // use while-loop to avoid spurious-wakeup problem
            while(queue_.empty()) {
                notEmpty_.wait();
            }
            assert(!queue_.empty());

            T front(std::move(queue_.front())); // 这里使用右值引用，当T是一个含有大量堆内存的对象时，可以节省大量的时间
            queue_.pop_front();
            return front;
        }

        queue_type drain() {
            std::deque<T> queue;

            {
                MutexLockGuard lock(mutex_);
                queue = std::move(queue_); // 调用deque的移动赋值函数
                assert(queue_.empty());
            }
        }

        size_t size() const {
            MutexLockGuard lock(mutex_);
            return queue_.size();
        }    

    private:  
        mutable MutexLock mutex_;
        Condition         notEmpty_ GUARDED_BY(mutex_);
        queue_type        queue_ GUARDED_BY(mutex_);
};

}



#endif // BO_NET_BASE_BLOCKINGQUEUE_H