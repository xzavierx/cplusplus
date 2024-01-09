#include <chrono>
#include <exception>
#include <future>
#include <iostream>
#include <memory>
#include <ostream>
#include <setjmp.h>
#include <stdexcept>
#include <thread>
void setValue(std::promise<int> p) {
  // try{
  //   throw std::runtime_error("Ooooops!");
  // } catch(...){
  //   p.set_exception(std::current_exception());
  //   return ;
  // }
  std::cout << "before setting value" << std::endl;
  p.set_value(100);
  std::this_thread::sleep_for(std::chrono::seconds(1));
  std::cout << "end setting value" << std::endl;
}

void Promise() {
  std::promise<int> p;
  std::future<int> f = p.get_future();
  std::thread t(setValue, std::move(p));
  std::cout << "Waiting for the thread to set value..." << std::endl;
  std::cout << "Value set by the thread: " << f.get() << std::endl;
  t.detach();
}

void SharedPromise() {
  std::promise<int> promise;
  std::shared_future<int> future = promise.get_future();
  std::thread thread1(setValue, std::move(promise));

  auto fn = [=](std::shared_future<int> future) {
    try{
      int result = future.get();
      std::cout << "Result: " << result << std::endl;
    } catch (const std::exception& e) {
      std::cout << "Future error:" << e.what() << std::endl;
    }
  };

  std::thread thread2(fn, future);
  std::thread thread3(fn, future);

  thread1.join();
  thread2.join();
  thread3.join();
}

int main() {
  // Promise();
  SharedPromise();

}

