#pragma once

#include "Condition.h"
#include "Mutex.h"

namespace bo_net
{

class CountDownLatch : boost::noncopyable
{
    public:

    explicit CountDownLatch(int count);

    void wait();

    void countDown();

    int getCount() const;

    private:
    mutable MutexLock mutex_;
    Condition condition_ GUARDED_BY(mutex_);
    int count_ GUARDED_BY(mutex_);
};

}  // namespace bo_net






// #include <condition_variable>
// #include <mutex>

// #include <pthread.h>

// namespace bo_net 
// {

// class CountDownLatch : noncopyable
// {

//     public:
//         explicit CountDownLatch(int count);

//         void wait();
        
//         void countDown();

//         int getCount() const;

//     private:
//         mutable std::mutex mutex_; // mutable用在这里感觉用处不大，毕竟根本不需要修改互斥锁
//         std::condition_variable condition_;
//         int count_ ; 
// };

// }