## 问题
在没有任何约束的多处理器系统上，多个线程同时读或写数个变量时，一个线程能观测到变量值更改的顺序不同于另一个线程写他们的顺序。实际上，更改的顺序甚至能在多个读取线程间相异。一些类似的效果还能在单处理器系统上出现，因为内存模型允许编译器进行变换。
[Memory Reordering Caught in the Act](https://preshing.com/20120515/memory-reordering-caught-in-the-act/)

## 概念术语
### 先序于
  个人认为是理论上的代码执行顺序, 实际上执行顺序，谁也说不准
```cpp
  r1 = y.load(std::memory_order_relaxed); // A
  x.store(r1, std::memory_order_relaxed); // B
```
A先序于B;

### 宽松定序
宽松定序只能保证数据操作的原子性（读或写），无法保证多个数据间的操作顺序。
```cpp
  // 线程1
  r1 = y.load(std::memory_order_relaxed); // A
  x.store(r1, std::memory_order_relaxed); // B

  // 线程2
  r2 = x.load(std::memory_order_relaxed); //C
  y.store(r2, std::memory_order_relaxed); //D
```
D happens before A, B happens before C;

原子性，定序，同步。
释放-获取定序，同步仅建立在释放或获得同一原子变量的线程之间。其他线程可能看到与被同步线程的一者或两者相异的内存访问顺序。

### 释放-获取定序
A线程：memory_order_release，B线程：memory_order_acquire
若线程A中的一个原子存储被标以memory_order_release，而线程B中从同一变量的原子加载被标以memory_order_acquire，且线程B中的加载读取到了线程A中的存储所写入的值，则线程A中的存储同步于线程B中的加载。
A线程发生在原子存储前的内存写入，在线程B中观察到A的新内存内容;
同步仅建立在释放和获得同一原子变量的线程之间。其他线程可能看到与被同步线程的一者或两者相异的内存访问顺序。

### 释放-消费定序
A线程：std::memory_order_release， B线程：std::memory_order_consume 
与释放-获取定序在与读进行了优化：
* std::memory_order_acquire要求后面所有的读都不得提前, 及之前的所有变量
* std::memory_order_consume是要求后面依赖于该操作的读不能乱序，一个是针对所有读，一个只是依赖于使用的这个变量
```cpp
// acquire
void producer()
{
  std::string* p = new std::string("Hello");
  data = 42;
  ptr.store(p, std::memory_order_release);
}
 
void consumer()
{
  std::string* p2;
  while (!(p2 = ptr.load(std::memory_order_acquire)))
    ;
  assert(*p2 == "Hello"); // 绝无问题
  assert(data == 42); // 绝无问题
}
// consume
void producer()
{
  std::string* p = new std::string("Hello");
  data = 42;
  ptr.store(p, std::memory_order_release);
}
 
void consumer()
{
  std::string* p2;
  while (!(p2 = ptr.load(std::memory_order_consume)))
      ;
  assert(*p2 == "Hello"); // 绝无出错： *p2 从 ptr 携带依赖
  assert(data == 42); // 可能也可能不会出错： data 不从 ptr 携带依赖
}
```

### 序列一致定序
被标为 memory_order_seq_cst 的原子操作不仅以与释放-获得定序相同的方式进行内存定序（在一个线程中先发生于存储的任何副作用都变成进行加载的线程中的可见副作用），
还对所有带此标签的内存操作建立了一个单独全序
