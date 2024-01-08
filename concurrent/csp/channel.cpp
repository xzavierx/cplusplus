#include <chrono>
#include <iostream>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <vcruntime.h>

template <typename T>
class Channel {
private:
  std::queue<T> _queue;
  std::mutex _mtx;
  std::condition_variable _cv_producer;
  std::condition_variable _cv_consumer;
  size_t _capacity;
  bool _closed = false;

public:
  Channel(size_t capacity = 0) : _capacity(capacity) { }

  bool send(T value) {
    std::unique_lock<std::mutex> lock(_mtx);
    _cv_producer.wait(lock, [this]{
      return (_capacity == 0 && _queue.empty()) || _queue.size() < _capacity || _closed;
    });
    if (_closed) {
      return false;
    }
    _queue.push(value);
    _cv_consumer.notify_one();
    return true;
  }

  bool receive(T &value) {
    std::unique_lock<std::mutex> lock(_mtx);
    _cv_consumer.wait(lock, [this]{ return !_queue.empty() || _closed; });
    if (_closed && _queue.empty()) {
      return false;
    }
    value = _queue.front();
    _queue.pop();
    _cv_producer.notify_one();
    return true;
  }

  void close() {
    std::unique_lock<std::mutex> lock(_mtx);
    _closed = true;
    _cv_producer.notify_all();
    _cv_consumer.notify_all();
  }
};

int main() {
  Channel<int> ch(10);
  std::thread producer([&]{
    for(int i = 0; i < 5; ++i) {
      ch.send(i);
      std::cout << "send: " << i << std::endl;
    }
    ch.close();
  });

  std::thread consumer([&]{
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    int v;
    while (ch.receive(v)) {
      std::cout << "recevied: " << v << std::endl;
    }
  });
  producer.join();
  consumer.join();
  return 0;
}