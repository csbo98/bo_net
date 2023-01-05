#include "CountDownLatch.h"

using namespace bo_net;

CountDownLatch::CountDownLatch(int count)
    : mutex_(),
      condition_(),
      count_(count)
{
}

void CountDownLatch::wait() {
    std::unique_lock<std::mutex> lock(mutex_);
    condition_.wait(lock, [&]() {return count_ == 0;});
}

void CountDownLatch::countDown() {
    std::lock_guard<std::mutex> lock(mutex_);
    --count_;
    
    if(count_ == 0) {
        condition_.notify_all(); // 加锁时同样可以notify条件变量的waiter，只不过可能开销比不加锁高
    }
}

int CountDownLatch::getCount() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return count_;
}