// author: bo liu(learn from muduo)
// Date: 2022-09-28 23:21:00

# pragma once

#include "copyable.h"
#include "Types.h"

namespace bo_net {
    
// UTC时间表示的时间戳，微秒解析度
// 这个结构体，去掉对boost的依赖
class Timestamp : public bo_net::copyable // 所有的命名空间在定义后可以直接使用，就像std
{
    public:
        // just construct a invalid Timestamp
        Timestamp():microSecondsSinceEpoch_(0) {}

        // construct a timestamp for given time
        explicit Timestamp(int64_t microSecondsSinceEpochArg) 
            : microSecondsSinceEpoch_(microSecondsSinceEpochArg) 
        { 
        }


        bool valid() const { return microSecondsSinceEpoch_ > 0; }

        // 使用标准库算法交换两个时间戳
        void swap(Timestamp& that)
        {
            std::swap(microSecondsSinceEpoch_, that.microSecondsSinceEpoch_);  // 普通的函数引用参数可以直接接收类成员变量作为参数
        }        
        
        // default copy/assignment/dtor are Okay
        // 转化为指定格式的字符串【多少秒.多少微秒】
        string toString() const;
        // 转化为标准的时间格式
        string toFormattedString(bool showMicroseconds = true) const;        

        // just for internal use
        int64_t microSecondsSinceEpoth() const {
            return microSecondsSinceEpoch_;
        }
        time_t secondsSinceEpoch() const
        { return static_cast<time_t>(microSecondsSinceEpoch_ / kMicroSecondsPerSecond); }

        
        // 下面几个函数都跟某个特定的对象无关，因此定义为静态成员函数
         
        // Get time of now. 返回当前时间的微秒
        static Timestamp now();

        //返回一个无效的 Timestamp 对象
        static Timestamp invalid()
        {
            return Timestamp();
        }

        static Timestamp fromUnixTime(time_t t)
        {
            return fromUnixTime(t, 0);
        }

        static Timestamp fromUnixTime(time_t t, int microseconds)
        {
            return Timestamp(static_cast<int64_t>(t) * kMicroSecondsPerSecond + microseconds);
        }


        static const int kMicroSecondsPerSecond = 1000 * 1000;
    private:
        int64_t microSecondsSinceEpoch_; // int64_t是long long int的别名
};


// 为TimeStamp重载<和==运算符，inline函数可以放在头文件中的原因是：
// 重载的运算符，可以放在类内，也可以放在类外。放在类外的时候只需要参数类型是定义的类，并且
// 放在能让调用这些运算符的调用者找到这个运算符就可以
inline bool operator<(const Timestamp& lhs, const Timestamp& rhs) {
    return lhs.microSecondsSinceEpoth() < rhs.microSecondsSinceEpoth();
} 

inline bool operator==(const Timestamp& lhs, const Timestamp& rhs) {
    return lhs.microSecondsSinceEpoth() == rhs.microSecondsSinceEpoth();
}

// inline函数必须对编译器可见，以便能够在调用点展开该函数，所以inline函数最好定义在头文件中
// inline函数是可以在多个文件中有多个相同的定义的

inline double timeDifference(Timestamp high, Timestamp low) {
    int64_t diff = high.microSecondsSinceEpoth() - low.microSecondsSinceEpoth(); 
    return static_cast<double>(diff) / Timestamp::kMicroSecondsPerSecond;
}

inline Timestamp addTime(Timestamp timestamp, double seconds) {
    int64_t delta = static_cast<int64_t>(seconds * Timestamp::kMicroSecondsPerSecond);
    return Timestamp(timestamp.microSecondsSinceEpoth() + delta);
}

} // namespace bo_net


