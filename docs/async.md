# async用法
std::async返回一个std::future
```cpp
  std::string fetchDataFromDB(std::string query) {}
  std::future<std::string> resultFromDB = std::async(std::launch::async, fetchDataFromDB, "Data");
  // 从future对象中获取数据,阻塞async的子线程结束
  std::string dbData = resultFromDB.get();
```

# async启动策略
1. std::launch::async: 再调用std::async后就开始执行
2. std::launch::deferred: 这种策略意味着将在调用std::future::get()或std::future::wait()函数时延迟执行，换句话说，任务将在需要结果时同步执行
3. std::launch::async | std::launch::deferred: 任务可在一个单独线程上异步执行，也可以延迟执行，编译器会计算当前能否开辟线程，如果能够则使用std::launch::async模式开辟线程，如果不能则采用std::launch::deffered串行执行。
std::async函数的默认策略为第三种，需要注意的时是，不同编译器和操作系统会有不同的默认行为

# std::future::get() vs. std::future::wait()
std::future::get()是一个阻塞调用给，用于获取std::future对象表示的值或异常，如果异步任务没有完成，get()会阻塞，get()只能调用一次，因为它会移动或消耗掉std::future对象的状态。
std::future::wait()也是一个阻塞调用，但它与get()的主要区别在于wait()不会返回任务的结果，它只是等待异步任务完成。如果任务已经完成，wait()会立即返回，如果任务没有完成wait()会阻塞当前线程，直到任务完成。

# std::future vs. std::shared_future
std::future::get() 只能调用一次，而std::shared_future则可以多次调用，不会消耗内部状态