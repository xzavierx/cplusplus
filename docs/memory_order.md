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
若线程A中的一个原子存储被标以memory_order_release，而线程B中从同一变量的原子加载被标以memory_order_acquire，且线程B中的加载读取到了线程A中的存储所写入的值，则线程A中的存储同步于线程B中的加载。
从线程A的视角先发生于原子存储的所有内存写入（包括非原子及宽松原子的），在线程B中成为可见副效应。即一旦原子加载完成，则线程B能观察到线程A写入内存的所有内容。
同步仅建立在释放和获得同一原子变量的线程之间。其他线程可能看到与被同步线程的一者或两者相异的内存访问顺序。

### 释放-消费定序
与释放-获取定序在与读进行了优化：
* std::memory_order_acquire要求后面所有的读都不得提前
* std::memory_order_consume是要求后面依赖于该操作的读不能乱序，一个是针对所有读，一个只是依赖于consume这条语句的读
