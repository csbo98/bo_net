
#include "Timestamp.h"  // include和在本文件直接声明函数与相关变量是一样的

#include <sys/time.h>
#include <stdio.h> // 标准IO库直接使用C语言的，C++的标准输入输出速度有点慢，可以通过取消与stdin和stdout的同步来增加cin/cout的速度


// https://blog.csdn.net/win_lin/article/details/7912693
// 使用PRId64，在包含头文件之前，需要首先定义这样的一个宏，之所以使用PRId64，是为了跨平台
//
#ifndef __STDC_FORMAT_MACROS
#define __STDC_FORMAT_MACROS
#endif

#include <inttypes.h>

// 在实现命名空间中函数的定义时，可以重新打开命名空间定义函数，也可以使用本文件中的方式在命名空间外部的全局作用域中定义
using namespace bo_net; // namesapce bo_net在各个功能的头文件中都有定义，那么include了某一个功能的头文件之后，namespace bo_net就在当前文件中可见了，可以直接使用；如果不include任何含有namespace bo_net的头文件，那么在当前的文件中是找不到该name的

// 编译时断言，在编译时计算条件是否成立。
static_assert(sizeof(Timestamp) == sizeof(int64_t), "Timestamp should be same size as int64_t");  // 类对象的大小只包括成员变量，不包括静态成员和函数，他们仅仅是在类这个作用域内罢了；在有虚函数的情况下，类的第一个成员其实是虚函数表指针


// 所谓的微秒解析度的时间戳，就是直接使用gettimeofday()函数获取到的时间精度可以达到微妙
Timestamp Timestamp::now()
{
  struct timeval tv;
  gettimeofday(&tv, NULL);  // time相关的函数中，获取时间点的常见函数除了gettimeofday，还有time、clock_gettime等；chrono库里面获取时间点的函数基本就靠system和steady两个时钟函数；另外可以通过chrono库计算出时间间隔，但是时间格式的转换都只能通过类C函数实现
  int64_t seconds = tv.tv_sec;
  return Timestamp(seconds * kMicroSecondsPerSecond + tv.tv_usec);
}

string Timestamp::toString() const
{
  char buf[32] = {0};
  int64_t seconds = microSecondsSinceEpoch_ / kMicroSecondsPerSecond;
  int64_t microseconds = microSecondsSinceEpoch_ % kMicroSecondsPerSecond;
  snprintf(buf, sizeof(buf), "%" PRId64 ".%06" PRId64 "", seconds, microseconds);
  return buf;
}

// 把纪元时转换为tm结构体，再转换为格式化字符串：time() -> gmtime_r() -> snprintf()
string Timestamp::toFormattedString(bool showMicroseconds) const
{
  char buf[64] = {0}; // 必须初始化
  time_t seconds = static_cast<time_t>(microSecondsSinceEpoch_ / kMicroSecondsPerSecond);
  struct tm tm_time;             // python中同样叫tm
  gmtime_r(&seconds, &tm_time);  // 把当前的纪元时间转化为一个时间结构体，这个函数返回的是世界协调时间

  if (showMicroseconds)
  {
    int microseconds = static_cast<int>(microSecondsSinceEpoch_ % kMicroSecondsPerSecond);
    snprintf(buf, sizeof(buf), "%4d%02d%02d %02d:%02d:%02d.%06d",
             tm_time.tm_year + 1900, tm_time.tm_mon + 1, tm_time.tm_mday,
             tm_time.tm_hour, tm_time.tm_min, tm_time.tm_sec,
             microseconds);
  }
  else
  {
    snprintf(buf, sizeof(buf), "%4d%02d%02d %02d:%02d:%02d",
             tm_time.tm_year + 1900, tm_time.tm_mon + 1, tm_time.tm_mday,
             tm_time.tm_hour, tm_time.tm_min, tm_time.tm_sec);
  }
  return buf;
}
