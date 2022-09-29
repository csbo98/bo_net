// author: bo liu(learn from muduo)
// Date: 2022-09-28 23:21:00

#ifndef BO_NET_BASE_COPYABLE_H
#define BO_NET_BASE_COPYABLE_H


namespace bo_net {
    // copyable作为一个标签类，用于强调对象是一个可拷贝对象  
    // 一个子类通过继承copyable类来强调自己是一个可拷贝对象，
    // 类的使用者如何知道呢

    // https://blog.csdn.net/sevenjoin/article/details/82222895
    // 构造函数不能被子类继承，上述链接讲解了构造函数的各种情况
    class copyable {
        protected:
            copyable() = default;
            ~copyable() = default;
    };
} // namespace bo_test


#endif
