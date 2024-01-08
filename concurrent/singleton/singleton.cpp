#include <iostream>
#include <memory>
#include <mutex>
// 在C++11以前在多线程环境下存在问题，多个线程同时访问， 存在多个线程初始化静态变量的情况
// C++11以后不存在,要求编译器保证内部静态变量的线程安全性
class Static {
private:
  Static() = default;
  Static(const Static&) = delete;
  Static& operator=(const Static&) = delete;
public:
  static Static& instance() {
    static Static s;
    return s;
  }
};
// 饿汉式，在程序启动后优先初始化
class Eager {
private:
  Eager() = default;
  Eager(const Eager&) = delete;
  Eager& operator=(const Eager&) = delete;

public:
  static Eager& Instance() {
    instance = new Eager;
    return *instance;
  }

private:
  static Eager* instance;
};
Eager* Eager::instance = nullptr;

// 懒汉式，不需要优先初始化，由用户决定什么时候初始化
// 代码问题，instance = new Lazy()操作
// 1. 调用allocate分配内存
// 2. 调用构造函数初始化对象
// 3. 将地址返回并赋值
// 编译器存在优化，执行顺序发生颠倒，1-2-3变成了1-3-2时，instance还未初始化好，就返回，其他线程获取的指针指向了未初始化的对象。
// 在C++11以后，可以使用atomic<Lazy*> instance来解决
class Lazy {
private:
  Lazy() = default;
  Lazy(const Lazy&) = delete;
  Lazy& operator=(const Lazy&) = delete;
public:
  static Lazy* Instance() {
    if (instance != nullptr) {
      return instance;
    }
    mtx.lock();
    if (instance != nullptr) {
      mtx.unlock();
      return instance;
    }
    mtx.unlock();
    return instance;
  }
private:
  static Lazy* instance;
  static std::mutex mtx;
};
Lazy* Lazy::instance = nullptr;
std::mutex Lazy::mtx;

// std::call_once能保证不会出现懒汉式的问题?
class Once {
private:
  Once(){};
  Once(const Once&) = delete;
  Once& operator=(const Once&) = delete;

public:
  static std::shared_ptr<Once> Instance() {
    std::once_flag flag;
    std::call_once(flag, []{
      instance = std::shared_ptr<Once>(new Once);
    }); 
    return instance;
  }

private:
 static std::shared_ptr<Once> instance;
};
std::shared_ptr<Once> Once::instance = nullptr;

// 像SafeDeleter在Singleton中确定需要吗？
template <class T>
class SafeDeleter {
public:
  void operator()(T* sf) {
    std::cout << "safe deleted" << std::endl;
    delete sf;
  }
};

// 实现一个单例模板类
template<class T>
class Singleton {
protected:
  Singleton() = default;
  ~Singleton() = default;
  Singleton(const Singleton<T>&) = delete;
  Singleton& operator=(const Singleton<T>&) = delete;

public:
  static std::shared_ptr<T> Instance(){
    static std::once_flag flag;
    std::call_once(flag, [&]{
      // 这里不能用make_shared，因为构造函数没有对shared_ptr开放
      instance = std::shared_ptr<T>(new T, safeDelete);
    });
    return instance;
  }

private:
  static void safeDelete(T* sf) {
    std::cout << "safe deleted" << std::endl;
    delete sf;
  }

private:
  static std::shared_ptr<T> instance;
};
template <class T>
std::shared_ptr<T> Singleton<T>::instance;

class Application: public Singleton<Application> {
  friend class Singleton<Application>;
private:
  Application() = default;
  ~Application() = default;
}; 

int main() {
  auto app = Application::Instance();
}