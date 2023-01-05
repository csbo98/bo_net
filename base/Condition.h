# pragma once

#include "Mutex.h"

#include <pthread.h>

namespace bo_net
{

class Condition :noncopyable
{
    public:

        explicit Condition(MutexLock &mutex)
            : mutex_(mutex) 
        {
            MCHECK(pthread_cond_init(&pcond_, NULL));
        }

        ~Condition() {
            MCHECK(pthread_cond_destroy(&pcond_));
        }

        void wait() { 
            MutexLock::UnassignGuard ug(mutex_); // Condition是MutexLock的友元类，这里使用RAII的方式来修改MutexLock里面的holder_
            MCHECK(pthread_cond_wait(&pcond_, mutex_.getPthreadMutex()));  // 注意这个系统调用需要获取到MutexLock的底层pthread锁
        }

        bool waitForSeconds(double seconds);

        void notifyOne() {
            MCHECK(pthread_cond_signal(&pcond_));
        }

        void notifyAll() {
            MCHECK(pthread_cond_broadcast(&pcond_));
        }

    private:

        MutexLock &mutex_;  // 因为要处理MutexLock的holder_，所以需要在Condition里面事先就拿到对应的MutexLock对象
                            // 一般情况下，条件变量在定义的时候已经比较明确和哪一个互斥锁协作，因此直接在初始化时就直接拿到对应的互斥锁也可以
        pthread_cond_t pcond_;
};

} // namespace bo_net