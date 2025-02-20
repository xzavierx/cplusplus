#include <atomic>
#include <iostream>
#include <memory>
#include <mutex>
#include <thread>
#include <cassert>
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
    // instance先初始化，指向的内存还未构建完成，那么就存在问题
    if (instance != nullptr) {
      return instance;
    }
    // 存在系统调用和上下文切换，这样会导致较大的开销
    mtx.lock();
    if (instance != nullptr) {
      mtx.unlock();
      return instance;
    }
    // instance先赋值，然后再分配内存，那么第二个线程会在第一次检查时直接获取到instance地址，
    instance = new Lazy();
    mtx.unlock();
    return instance;
  }
private:
  static Lazy* instance;
  static std::mutex mtx;
};
Lazy* Lazy::instance = nullptr;
std::mutex Lazy::mtx;

class LockFree {
private:
  LockFree() = default;
  LockFree(const LockFree&) = delete;
  LockFree& operator=(const LockFree&) = delete;

public:
  static LockFree* Instance() {
    // 无锁检查：通过 instance.load(std::memory_order_acquire) 快速检查实例是否已创建
    // 低开效：通常只需几条CPU指令，开销非常低, 而且通常时非阻塞的，不会导致线程挂起或上下文切换, 比mutex至少小几十倍
    LockFree* p = nullptr; 
    if ((p = instance.load(std::memory_order_acquire))) {
      return p;
    }
    // 加锁创建：确保只有一个实例创建实例
    std::lock_guard<std::mutex> lock(mtx);
    if ((p = instance.load(std::memory_order_relaxed))) {
      std::cout << "load ok" << std::endl;
      assert(p != nullptr);
      return p;
    }
    p = new LockFree;
    // 创建实例后，使用 instance.store(p, std::memory_order_release) 原子地存储实例指针
    instance.store(p, std::memory_order_release);
    return p;
  }

  static void TestLockFree() {
    std::thread t1([]{
      Instance();
    });
    std::thread t2([]{
      Instance();
    });
    t1.join();t2.join();
    delete Instance();
    instance.store(nullptr, std::memory_order_relaxed);
  }

private:
  static std::atomic<LockFree*> instance;
  static std::mutex mtx;
};
std::atomic<LockFree*> LockFree::instance;
std::mutex LockFree::mtx;

// std::call_once能保证不会出现懒汉式的问题?
// 2024-03-06 std::call_once解决了懒汉式的问题
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
public:
  static void Test() {
    auto app = Application::Instance();
  }
}; 

int main() {
  while(true)
    LockFree::TestLockFree();
}