#include <atomic>
#include <string>
#include <thread>
#include <cassert>
#include <iostream>

std::atomic<std::string*> ptr;
int data;
void test_release_consume(){
  std::thread t1([&]{
    std::string* p = new std::string("Hello");
    data = 42;
    ptr.store(p, std::memory_order_release);
  });

  std::thread t2([&]{
    std::string *p2;
    while(!(p2 = ptr.load(std::memory_order_consume))) {
      std::cout << "loading" << p2 <<std::endl;
    };
    assert(*p2 == "Hello");
    assert(data == 42);
  });
  t1.join(); t2.join();
}

int main() {
  test_release_consume();
}
