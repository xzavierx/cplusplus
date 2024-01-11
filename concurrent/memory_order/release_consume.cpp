#include <atomic>
#include <string>
#include <thread>
#include <cassert>
#include <iostream>
#include <vcruntime.h>

void test_release_consume(){
  std::atomic<std::string*> ptr;
  int data;
  std::thread t1([&]{
    std::string* p = new std::string("Hello");
    data = 42;
    ptr.store(p, std::memory_order_release);
  });

  std::thread t2([&]{
    std::string *p2;
    while(!(p2 = ptr.load(std::memory_order_consume)));
    assert(*p2 == "Hello");
    assert(data == 42);
  });
  t1.join(); t2.join();
}

void test_release_consume2() {
  int Playload = 0;
  std::atomic<int*> Guard(nullptr); 
  std::thread t1([&]{
    Playload = 42;
    Guard.store(&Playload, std::memory_order_release);
  });
  std::thread t2([&]{
    auto gg = &Playload;
    while (!Guard.load(std::memory_order_consume));
    if (*gg != 42) {
      std::cout << *gg << std::endl; 
    }
  });  
  t1.join(); t2.join();
}

void test_release_consume3() {
  std::atomic<int> Guard;
  std::thread t1([&]{
    Guard.store(1, std::memory_order_release);
  });
  std::thread t2([&]{
    Guard.fetch_add(1, std::memory_order_relaxed);
  });
  std::thread t3([&]{
    auto g = Guard.load(std::memory_order_consume);
    std::cout << g << std::endl;
  });
  t1.join(), t2.join(), t3.join();
}

int main() {
  // std::cout << ptr.is_lock_free() << std::endl;
  // test_release_consume();
  while(true)
    test_release_consume();
}
