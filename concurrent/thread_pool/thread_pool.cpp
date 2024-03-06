#include "thread_pool.h"

int main() {
  int m = 0;
  auto future = ThreadPool::instance().commit([](int& m){
    m = 1024;
    std::cout << "in thread m:" << m << std::endl;
  }, std::ref(m));
  future.get();
  std::cout << "in main thread, m:" << m << std::endl;
}