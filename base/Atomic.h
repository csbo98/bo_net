#ifndef BO_NET_BASE_ATOMIC_H
#define BO_NET_BASE_ATOMIC_H

#include <boost/noncopyable.hpp>
#include <stdint.h>

/**
 * https://gcc.gnu.org/onlinedocs/gcc-4.1.0/gcc/Atomic-Builtins.html
 * 上述文档是gcc对常见的原子操作的介绍
*/

namespace bo_net 
{

namespace detail 
{

template<typename T> 
class AtomicIntegerT : boost::noncopyable
{
    public:
        AtomicIntegerT()
            : value_(0) 
        {
        }
        
        // AtomicIntegerT(const AtomicIntegerT& that)
        //   : value_(that.get())
        // {}
        
        // AtomicIntegerT& operator=(const AtomicIntegerT& that)
        // {
        //   getAndSet(that.get());
        //   return *this;
        // }
    
        T get() {
            // 这个函数一定会返回value_的原值，因此可以用于设计读取操作
            return __sync_val_compare_and_swap(&value_, 0, 0);
        }   

        T getAndAdd(T x) {
            return __sync_fetch_and_add(&value_, x);
        }

        T addAndGet(T x) {
            return getAndAdd(x) + x;
        }

        T incrementAndGet() {
            return addAndGet(1);
        }

        T decrementAndGet() {
            return addAndGet(-1);
        }

        void add(T x){
            getAndAdd(x);
        }

        void increment(){
            incrementAndGet();
        }

        void decrement(){
            decrementAndGet();
        }

        T getAndSet(T newValue) {
            return __sync_lock_test_and_set(&value_, newValue);
        }

    private:
        volatile T value_; // volatile主要是为了防止编译器考虑不到的那些情况，多线程程序中，如果一个变量被多个线程使用，那么编译器本身就不会优化，这里添加这个标志，只是增加一个保证
};

} // namespace detail

typedef detail::AtomicIntegerT<int32_t> AtomicInt32;
using AtomicInt64 = detail::AtomicIntegerT<int64_t>;

} // namespace bo_net


#endif