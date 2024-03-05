#include <algorithm>
#include <atomic>
#include <iterator>
#include <iostream>
#include <vector>
#include <thread>
void test_relaxed() {
  std::atomic<int> a{0};
  std::vector<int> v3, v4;
  std::thread t1([&]{
    for(auto i = 1; i < 10; ++i){
      a.store(i, std::memory_order_relaxed);
    }
  });
  std::thread t2([&]{
    for(auto i = 1; i < 10; ++i){
      a.store(i, std::memory_order_relaxed);
    }
  });
  std::thread t3([&]{
    for (auto i = 1; i < 10; ++i) {
      v3.push_back(a.load(std::memory_order_relaxed));
    }
  });
  std::thread t4([&]{
    for (auto i = 1; i < 10; ++i) {
      v4.push_back(a.load(std::memory_order_relaxed));
    }
  });
  t1.join(), t2.join(), t3.join(), t4.join();
  if (std::count(v3.begin(), v3.end(), 9) != 9) {
    std::copy(v3.begin(), v3.end(), std::ostream_iterator<int>(std::cout, " "));
    std::cout << std::endl;
  }
  if (std::count(v4.begin(), v4.end(), 9) != 9) {
    std::copy(v4.begin(), v4.end(), std::ostream_iterator<int>(std::cout, " "));
    std::cout << std::endl;
  }
}
int main() {
  for (int i = 0 ; i < 100000; ++i)
    test_relaxed();
}