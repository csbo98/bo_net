#ifndef BO_NET_BASE_MUTEX_H
#define BO_NET_BASE_MUTEX_H

#include "CurrentThread.h"
#include "noncopyable.h"

#include <assert.h>
#include <pthread.h>

// Thread safety annotations {
// https://clang.llvm.org/docs/ThreadSafetyAnalysis.html

// Enable thread safety attributes only with clang.
// The attributes can be safely erased when compiling with other compilers.
#if defined(__clang__) && (!defined(SWIG))
#define THREAD_ANNOTATION_ATTRIBUTE__(x)   __attribute__((x))
#else
#define THREAD_ANNOTATION_ATTRIBUTE__(x)   // no-op
#endif

#define CAPABILITY(x) \
  THREAD_ANNOTATION_ATTRIBUTE__(capability(x))

#define SCOPED_CAPABILITY \
  THREAD_ANNOTATION_ATTRIBUTE__(scoped_lockable)

#define GUARDED_BY(x) \
  THREAD_ANNOTATION_ATTRIBUTE__(guarded_by(x))

#define PT_GUARDED_BY(x) \
  THREAD_ANNOTATION_ATTRIBUTE__(pt_guarded_by(x))

#define ACQUIRED_BEFORE(...) \
  THREAD_ANNOTATION_ATTRIBUTE__(acquired_before(__VA_ARGS__))

#define ACQUIRED_AFTER(...) \
  THREAD_ANNOTATION_ATTRIBUTE__(acquired_after(__VA_ARGS__))

#define REQUIRES(...) \
  THREAD_ANNOTATION_ATTRIBUTE__(requires_capability(__VA_ARGS__))

#define REQUIRES_SHARED(...) \
  THREAD_ANNOTATION_ATTRIBUTE__(requires_shared_capability(__VA_ARGS__))

#define ACQUIRE(...) \
  THREAD_ANNOTATION_ATTRIBUTE__(acquire_capability(__VA_ARGS__))

#define ACQUIRE_SHARED(...) \
  THREAD_ANNOTATION_ATTRIBUTE__(acquire_shared_capability(__VA_ARGS__))

#define RELEASE(...) \
  THREAD_ANNOTATION_ATTRIBUTE__(release_capability(__VA_ARGS__))

#define RELEASE_SHARED(...) \
  THREAD_ANNOTATION_ATTRIBUTE__(release_shared_capability(__VA_ARGS__))

#define TRY_ACQUIRE(...) \
  THREAD_ANNOTATION_ATTRIBUTE__(try_acquire_capability(__VA_ARGS__))

#define TRY_ACQUIRE_SHARED(...) \
  THREAD_ANNOTATION_ATTRIBUTE__(try_acquire_shared_capability(__VA_ARGS__))

#define EXCLUDES(...) \
  THREAD_ANNOTATION_ATTRIBUTE__(locks_excluded(__VA_ARGS__))

#define ASSERT_CAPABILITY(x) \
  THREAD_ANNOTATION_ATTRIBUTE__(assert_capability(x))

#define ASSERT_SHARED_CAPABILITY(x) \
  THREAD_ANNOTATION_ATTRIBUTE__(assert_shared_capability(x))

#define RETURN_CAPABILITY(x) \
  THREAD_ANNOTATION_ATTRIBUTE__(lock_returned(x))

#define NO_THREAD_SAFETY_ANALYSIS \
  THREAD_ANNOTATION_ATTRIBUTE__(no_thread_safety_analysis)

// End of thread safety annotations }

#ifdef CHECK_PTHREAD_RETURN_VALUE

#ifdef NDEBUG
__BEGIN_DECLS
extern void __assert_perror_fail (int errnum,
                                  const char *file,
                                  unsigned int line,
                                  const char *function)
    noexcept __attribute__ ((__noreturn__));
__END_DECLS
#endif

#define MCHECK(ret) ({ __typeof__ (ret) errnum = (ret);         \
                       if (__builtin_expect(errnum != 0, 0))    \
                         __assert_perror_fail (errnum, __FILE__, __LINE__, __func__);})

#else  // CHECK_PTHREAD_RETURN_VALUE

#define MCHECK(ret) ({ __typeof__ (ret) errnum = (ret);         \
                       assert(errnum == 0); (void) errnum;})

#endif // CHECK_PTHREAD_RETURN_VALUE

namespace bo_net 
{

// is a wrapper of pthread_mutex_t, usually, used as data member of a class, eg.
//
// class Foo
// {
//  public:
//   int size() const;
//
//  private:
//   mutable MutexLock mutex_;
//   std::vector<int> data_ GUARDED_BY(mutex_);
// };
class CAPABILITY("mutex") MutexLock : noncopyable
{
    public:

        MutexLock()
            : holder_(0) // 初始化时，没有任何线程拥有锁
        {
            MCHECK(pthread_mutex_init(&mutex_, NULL)); // 初始化锁
        }

        ~MutexLock() {
            assert(holder_ == 0); // 只有在当前没有任何线程持有锁的情况下，才可以销毁锁；否则程序必然设计的有问题，某一个线程仍然在访问锁保护的对象，但是另外一个线程却要析构锁对象
            MCHECK(pthread_mutex_destroy(&mutex_));
        }

        bool isLockedByThisThread() const
        {
            return holder_ == CurrentThread::tid();
        }
        // 调用时，如果当前线程不持有锁，那么程序终止
        void assertLocked() const ASSERT_CAPABILITY(this)
        {
            assert(isLockedByThisThread());
        }        

        void lock() ACQUIRE() {
            MCHECK(pthread_mutex_lock(&mutex_));
            assignHolder(); // 必须先加锁，然后才可以把当前线程的tid赋值给holder_
        }

        void unlock() RELEASE() {
            unassignHolder();  // 必须在持有锁的时候才可以修改holder_
            MCHECK(pthread_mutex_unlock(&mutex_));
        }

        pthread_mutex_t* getPthreadMutex() {
            return &mutex_;   // 返回锁的地址，主要是给Condition类使用
        }

    private:
        friend class Condition; // 友元类声明，不受private和public的影响；Condition和Mutex会同时使用，Condition会用到Mutex的holder_，因此，声明为友元类

        class UnassignGuard : noncopyable
        {
            public:
                explicit UnassignGuard(MutexLock &owner)
                    : owner_(owner)
                {
                    owner_.unassignHolder();
                }

                ~UnassignGuard() {
                    owner_.assignHolder();
                }

            private:
                MutexLock &owner_;
        };    

        void assignHolder() {
            holder_ = CurrentThread::tid(); // 把当前线程的tid赋值给holder_，这个函数在线程lock得到锁之后调用
        }

        void unassignHolder() {
            holder_ = 0;        // 将锁标记为未被任何线程持有，这个函数需要在线程释放锁之前调用
        }

        pthread_mutex_t mutex_;
        pid_t holder_;          // 标记锁当前被哪个线程持有，这个标记变量同样被锁保护着 
};

// 使用RAII思想实现一个对上述互斥锁的封装类，使用此类来加锁，相比于直接使用lock和unlock更加安全
// Use as a stack variable, eg.
// int Foo::size() const
// {
//   MutexLockGuard lock(mutex_);
//   return data_.size();
// }

class SCOPED_CAPABILITY MutexLockGuard : noncopyable
{   
    public:
        explicit MutexLockGuard(MutexLock &mutex) ACQUIRE(mutex) 
        : mutex_(mutex) 
        {
            mutex_.lock();
        }

        ~MutexLockGuard() RELEASE() 
        {
            mutex_.unlock();
        }
    private:
        MutexLock& mutex_; // MutexLock是不可以拷贝的，这里使用引用，就算一个类delete了拷贝构造函数和赋值函数，仍然可以使用引用
};

} // namespace bo_net

// 不能通过临时对象来使用RAII-style locking
// Prevent misuse like:
// MutexLockGuard(mutex_);
// A tempory object doesn't hold the lock for long!
#define MutexLockGuard(x) error "Missing guard object name"



#endif // BO_NET_BASE_MUTEX_H