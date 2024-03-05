#include <chrono>
#include <exception>
#include <future>
#include <iostream>
#include <memory>
#include <ostream>
#include <setjmp.h>
#include <stdexcept>
#include <thread>
// std::promise类似于golang的channel
void setValue(std::promise<int> p) {
  // try{
  //   throw std::runtime_error("Ooooops!");
  // } catch(...){
  //   p.set_exception(std::current_exception());
  //   return ;
  // }
  std::cout << "before setting value" << std::endl;
  // 设置promise的值，设置完成以后，主线程立即都能获取到值了
  p.set_value(100);
  std::this_thread::sleep_for(std::chrono::seconds(1));
  std::cout << "end setting value" << std::endl;
}
// 子线程抛出异常可以调用promise::set_exception将异常传递出去，
// 其他调用promise::get()的线程需要try-catch捕获这些异常

void Promise() {
  std::promise<int> p;
  std::future<int> f = p.get_future();
  std::thread t(setValue, std::move(p));
  std::cout << "Waiting for the thread to set value..." << std::endl;
  std::cout << "Value set by the thread: " << f.get() << std::endl;
  t.join();
}

void SharedPromise() {
  std::promise<int> promise;
  // 多个线程等待同一个异步操作的结果,
  // 此时shared_future::get调用多次都不会出现问题，应该不会消耗内部状态了
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

