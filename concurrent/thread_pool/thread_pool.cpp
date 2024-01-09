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

  template<class Fn, class... Args>
  auto commit(Fn&& fn, Args&&... args) -> std::future<decltype(fn(args...))> {
    using RetType = decltype(fn(args...));
    if (_stop.load()) {
      return std::future<RetType>{};
    }
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
  ThreadPool(unsigned int num = 5) : _stop(false) {
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

int main() {
  int m = 0;

  auto future = ThreadPool::instance().commit([](int& m){
    m = 1024;
    std::cout << "in thread m:" << m << std::endl;
  }, std::ref(m));
  future.get();
  std::cout << "in main thread, m:" << m << std::endl;
  return 0;
}