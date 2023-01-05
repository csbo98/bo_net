# pragma once

namespace bo_net
{

class noncopyable
{
 public:
 // delete 掉拷贝构造函数和赋值函数，可以作为一个不可复制的基类
  noncopyable(const noncopyable&) = delete;
  noncopyable& operator=(const noncopyable&) = delete;

 protected:
  // 创建派生类对象时，默认会调用这个构造函数
  noncopyable() = default;
  ~noncopyable() = default;
};

}  // namespace bo_net