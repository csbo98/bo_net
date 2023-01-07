# pragma once

namespace bo_net
{

class noncopyable
{
 public:
 // delete 掉拷贝构造函数和赋值函数，可以作为一个不可复制的基类
 // 继承了这个类的派生类不可以再定义拷贝构造函数和拷贝赋值函数
 // 这样该类就具有不可拷贝构造和不可拷贝复制的性质了

 // 另一种方式是把类的拷贝构造函数和拷贝赋值函数都声明为private的，
 // 可以达到与delete相同的效果
  noncopyable(const noncopyable&) = delete;
  noncopyable& operator=(const noncopyable&) = delete;

 protected:
  // 创建派生类对象时，默认会调用这个构造函数，构造函数被声明成protected的，旨在说明
  // 这个类只能用做不可实例化的基类
  noncopyable() = default;
  ~noncopyable() = default;
};

}  // namespace bo_net