#pragma once
#include <algorithm>
#include <atomic>
#include <future>
#include <iostream>
#include <memory>
#include <mutex>
#include <queue>
#include <thread>
#include <utility>
#include <vector>
#include <iterator>

class ThreadPool{
public:
  ThreadPool(const ThreadPool&) = delete;
  ThreadPool& operator=(const ThreadPool&) = delete;


  static ThreadPool& instance() {
    static ThreadPool s;
    return s;
  }


  using Task = std::packaged_task<void()>;
  ~ThreadPool() {
    stop();
  }

  // 提交一个Task，同时返回一个std::future，用于获取任务的执行结果
  template<class Fn, class... Args>
  auto commit(Fn&& fn, Args&&... args) -> std::future<decltype(fn(args...))> {
    using RetType = decltype(fn(args...));
    // 如果线程池已停止，返回一个空的std::future，表示任务未被提交
    if (_stop.load()) {
      return std::future<RetType>{};
    }
    // 用于将可调用对象打包成一个任务，并允许获取其执行结果, task中保存执行结果
    auto task = std::make_shared<std::packaged_task<RetType()>>(
      std::bind(std::forward<Fn>(fn), std::forward<Args>(args)...));
    std::future<RetType> ret = task->get_future();
    {
      std::lock_guard<std::mutex> lock(_mtx);
      _tasks.emplace([=]{(*task)();});
    }
    _cv.notify_one();
    return ret;
  }

private:
  ThreadPool(unsigned int num = 1) : _stop(false) {
    _threadNum = num > 1 ? num : 1;
    start();
  }
  
  void start() {
    for (auto i= 0; i < _threadNum; ++i) {
      _pool.emplace_back([this]{
        while (!this->_stop.load()) {
          Task task;
          {
            std::unique_lock<std::mutex> lock(_mtx);
            this->_cv.wait(lock, [this]{
              return this->_stop.load() || !this->_tasks.empty();
            });
            // 任务队列为空，说明_stop为true了, 退出该线程循环
            if (this->_tasks.empty()) {
              return;
            }
            task = std::move(this->_tasks.front());
            this->_tasks.pop();
          }
          this->_threadNum--;
          task();
          this->_threadNum++;
        }
      });
    }
  }

  void stop() {
    _stop.store(true);
    _cv.notify_all();
    for (auto& td : _pool) {
      if (td.joinable()) {
        std::cout << "join thread: " << td.get_id() << std::endl;
        td.join();
      }
    }
  }

private:
  std::mutex _mtx;
  std::condition_variable _cv;
  std::atomic_bool _stop;
  std::atomic_int _threadNum;
  std::queue<Task> _tasks;
  std::vector<std::thread> _pool;
};
