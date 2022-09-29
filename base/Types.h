// author: bo liu(learn from muduo)
// Date: 2022-09-28 23:21:00


#pragma once

// #include应该出现在打开命名空间的操作之前
#include <stdint.h>
#include <string.h> // use memset to set memory to zero
#include <string>


#ifndef NDEBUG
#include <assert.h>
#endif

// 
namespace bo_net
{
    using std::string;

    // 将p指向的长度为n的内存区域所有字节设置为0
    // 一般只把inline函数或者static函数定义在头文件中
    inline void memZero(void *p, size_t n) {
        memset(p, 0, n);
    }


}