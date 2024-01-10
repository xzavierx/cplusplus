#include <atomic>
#include <iterator>
#include <string>
#include <cassert>
#include <thread>
#include <iostream>
#include <vector>

std::atomic<std::string*> ptr;
int data;
std::string data2;

void test_order_release() {
  std::thread t1([]{
    std::string* p = new std::string("Hello");
    data = 42;
    data2 = "ddd";
    ptr.store(p, std::memory_order_release);
  });
  std::thread t2([]{
    std::string* p2;
    while(!(p2 = ptr.load(std::memory_order_acquire)));
    assert(*p2 == "Hello");
    assert(data == 42);
    assert(data2 == "ddd");
  });
  t1.join();
  t2.join();
}

// 线程会卡住，因为必须要保证调用memory_order_release在前
void test_order_release2() {
  std::thread t3([=]{
    std::string* p2;
    while(!(p2 = ptr.load(std::memory_order_acquire)));
    std::cout << "load finished" << std::endl; 
  });
  t3.join();
}

void test_order_release3() {
  static std::vector<int> data;
  static std::atomic<int> flag;

  std::thread a([=]{
    data.push_back(42);
    flag.store(1, std::memory_order_release);
    std::cout << "flag stored" << std::endl;
  });
  std::thread b([=]{
    int expected = 1;
    while (!flag.compare_exchange_strong(expected, 2, std::memory_order_relaxed)){
      expected = 1;
    }
    std::cout << "flag compare and exchanged" << std::endl;
  });
  std::thread c([=]{
    while(flag.load(std::memory_order_acquire) < 2);
    std::cout << "flag loaded" << std::endl;
    assert(data.at(0) == 42);
  });
  a.join(); b.join(); c.join();
}

int main() {
  // test_order_release();
  test_order_release3();
}