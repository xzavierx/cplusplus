#include <limits.h>
#include <thread>
#include <utility>
#include <iostream>
class JoiningThread {
public:
  JoiningThread() noexcept = default;
  template<typename Callable, typename... Args>
  explicit JoiningThread(Callable&& func, Args&& ...args):
    _t(std::forward<Callable>(func), std::forward<Args>(args)...){}
  explicit JoiningThread(std::thread t) noexcept: _t(std::move(t)){}
  JoiningThread(JoiningThread &&other) noexcept: _t(std::move(other._t)){}
  JoiningThread& operator=(JoiningThread&& other) noexcept {
    // 先保证当前线程join，否则就奔溃
    if (joinable()) {
      join();
    }
    _t = std::move(other._t);
    return *this;
  }

  ~JoiningThread() noexcept{
    if (joinable()) {
      join();
    }
  }

  void swap(JoiningThread& other) noexcept {
    _t.swap(other._t);
  }

  std::thread::id Id() const noexcept {
    return _t.get_id();
  }

  bool joinable() const noexcept {
    return _t.joinable();
  }
  void join() {
    _t.join();
  }

  void detach() {
    _t.detach();
  }

  std::thread& asThread() noexcept{
    return _t;
  }

  const std::thread& asThread() const noexcept{
    return _t;
  }

private:
  std::thread _t;
};


int main() {
}