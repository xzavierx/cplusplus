#include <chrono>
#include <memory>
#include <mutex>
#include <queue>
#include <iostream>
#include <thread>
template <typename T>
class Queue {
public:
  Queue() = default;
  Queue(Queue const& other) {
    std::lock_guard<std::mutex> lock(other._mtx);
    _queue = other._queue;
  }
  void push(T value) {
    std::lock_guard<std::mutex> lock(_mtx);
    std::cout << std::this_thread::get_id() << "-" << "push:" << value << std::endl;
    _queue.push(value);
    _cv.notify_one();
  }
  // 这里传入引用的目的是让调用者分配内存，
  // 如果遇到内存不足的情况，能保证_queue里面的数据完整性，不会等pop了以后才遇到这个错误（针对T占用内存较大）,让数据落地
  void pop(T& value) {
    std::unique_lock<std::mutex> lock(_mtx);
    _cv.wait(lock, [this] {return !_queue.empty();});
    value = _queue.front();
    std::cout << std::this_thread::get_id() << "-" << "pop:" << value << std::endl;
    _queue.pop();
  }
  std::shared_ptr<T> pop() {
    std::unique_lock<std::mutex> lock(_mtx);
    _cv.wait(lock, [this]{return !_queue.empty();});
    std::shared_ptr<T> p(std::make_shared<T>(_queue.front()));
    std::cout << std::this_thread::get_id() << "-" << "pop:" << *p << std::endl;
    _queue.pop();
    return p;
  }
  bool tryPop(T &value) {
    std::lock_guard<std::mutex> lock(_mtx);
    if (_queue.empty()) {
      return false;
    }
    value = _queue.front();
    _queue.pop();
    return true;
  }
  std::shared_ptr<T> tryPop() {
    std::lock_guard<std::mutex> lock(_mtx);
    if (_queue.empty()) 
      return std::shared_ptr<T>();

    std::shared_ptr<T> p(std::make_shared<T>(_queue.front()));
    _queue.pop();
    return p;
  }
  bool empty() const {
    std::lock_guard<std::mutex> lock(_mtx);
    return _queue.empty();
  }
  
private:
  mutable std::mutex _mtx;
  std::queue<T> _queue;
  std::condition_variable _cv;
};

int main() {
  Queue<int> queue;
  std::mutex mtx;
  std::thread producer([&]{
    for (int i = 0; ; ++i) {
      queue.push(i);
      std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }
  });

  auto fn = [&]{
    for (;;) {
      auto data = queue.pop();
      std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    }
  };
  std::thread consumer1(fn);
  std::thread consumer2(fn);
  producer.join();
  consumer1.join();
  consumer2.join();
}